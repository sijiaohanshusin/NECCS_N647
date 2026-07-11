#include "app_camera.h"

#include "app_camera_display.h"
#include "app_camera_imx219.h"
#include "main.h"
#include <string.h>

#define APP_CAMERA_DCMIPP_PIPE          DCMIPP_PIPE1
#define APP_CAMERA_DCMIPP_VC            DCMIPP_VIRTUAL_CHANNEL0
#define APP_CAMERA_PRIMARY_PHY          DCMIPP_CSI_PHY_BT_900
#define APP_CAMERA_FALLBACK_PHY         DCMIPP_CSI_PHY_BT_950
#define APP_CAMERA_DATA_LANE_MAPPING    DCMIPP_CSI_PHYSICAL_DATA_LANES
#define APP_CAMERA_RAW10_FRAME_WORDS    (APP_CAMERA_RAW10_FRAME_BYTES / 4U)
#define APP_CAMERA_DPHY_RETRY_DELAY_MS  1000U
#define APP_CAMERA_DCACHE_LINE_BYTES    32U
#define APP_CAMERA_CAPTURE_OUTPUT_BPP   1U
#define APP_CAMERA_PREVIEW_OUTPUT_BPP   2U
#define APP_CAMERA_CAPTURE_OUTPUT_FORMAT DCMIPP_PIXEL_PACKER_FORMAT_MONO_Y8_G8_1
#define APP_CAMERA_PREVIEW_OUTPUT_FORMAT DCMIPP_PIXEL_PACKER_FORMAT_RGB565_1
#define APP_CAMERA_RAW_BAYER_TYPE       DCMIPP_RAWBAYER_RGGB
/* Edge-adaptive demosaic instead of pure bilinear: recovers luma detail and
 * removes the zipper/maze artefacts that flattened perceived contrast. */
#define APP_CAMERA_RAW_BAYER_STRENGTH   DCMIPP_RAWBAYER_ALGO_STRENGTH_8
#define APP_CAMERA_FRAME_SAMPLE_GRID    8U
#define APP_CAMERA_FRAME_DARK_LUMA      14U

/* --- ISP tuning (gray/washed-out fix, 2026-07-11) ---
 * The raw pipe used to be RAW10 -> bilinear demosaic -> RGB565 with no tone
 * or color processing at all, which renders flat and gray. The blocks below
 * add the missing minimum ISP chain. */
/* IMX219 data pedestal is 64 LSB @10 bit = 16 LSB in the ISP 8-bit domain. */
#define APP_CAMERA_ISP_BLACK_LEVEL      16U
/* AE servo: average luma target on the 0..187 scale of AppCamera_Rgb565Luma
 * (2*R5 + G6 + 2*B5); 92 is a mid-gray exposure. */
#define APP_CAMERA_AE_TARGET_LUMA       92U
#define APP_CAMERA_AE_DEADBAND          10U
#define APP_CAMERA_AE_MIN_EXPOSURE      64U
#define APP_CAMERA_AE_MAX_EXPOSURE      3400U
#define APP_CAMERA_AE_MAX_AGAIN_CODE    200U
#define APP_CAMERA_AE_MIN_DGAIN         0x0100U
#define APP_CAMERA_AE_MAX_DGAIN         0x0400U
/* White-balance gains in Q7 (128 = x1.0), applied in the DCMIPP exposure
 * block (pre-demosaic, per Bayer component). Startup values are a typical
 * IMX219 indoor operating point; the grey-world loop refines them. */
#define APP_CAMERA_WB_GAIN_MIN          80U
#define APP_CAMERA_WB_GAIN_MAX          1000U

DCMIPP_HandleTypeDef hdcmipp;

static volatile AppCameraStatus_t g_app_camera_status =
{
  .width = APP_CAMERA_WIDTH,
  .height = APP_CAMERA_HEIGHT,
  .fps = APP_CAMERA_FPS,
  .frame0_addr = APP_CAMERA_FRAME0_ADDR,
  .frame1_addr = APP_CAMERA_FRAME1_ADDR,
  .frame_bytes = APP_CAMERA_CAPTURE_FRAME_BYTES,
  .frame_buffer_bytes = APP_CAMERA_FRAME_BUFFER_BYTES,
  .output_format = APP_CAMERA_CAPTURE_OUTPUT_FORMAT,
  .output_bpp = APP_CAMERA_CAPTURE_OUTPUT_BPP,
  .line_pitch = APP_CAMERA_CAPTURE_LINE_BYTES,
  .bayer_type = APP_CAMERA_RAW_BAYER_TYPE,
  .completed_frame_addr = APP_CAMERA_FRAME0_ADDR,
  .display_addr = APP_CAMERA_DISPLAY_TARGET_ADDR,
  .pending_display_addr = APP_CAMERA_DISPLAY_TARGET_ADDR,
  .pipe_index = APP_CAMERA_DCMIPP_PIPE,
  .phy_bitrate = APP_CAMERA_PRIMARY_PHY,
};

static uint32_t g_app_camera_last_frame_poll;
static uint32_t g_app_camera_no_frame_ms;
static uint32_t g_app_camera_phy_retry_index;
static uint32_t g_app_camera_stream_recover_attempts;
static const uint32_t g_app_camera_phy_retry_list[] =
{
  APP_CAMERA_PRIMARY_PHY,
  APP_CAMERA_FALLBACK_PHY,
};

volatile uint32_t g_app_camera_state = 0U;
volatile uint32_t g_app_camera_init_status = 0U;
volatile uint32_t g_app_camera_start_status = 0U;
volatile uint32_t g_app_camera_chip_id = 0U;
volatile uint32_t g_app_camera_frame_count = 0U;
volatile uint32_t g_app_camera_csi_sof_count = 0U;
volatile uint32_t g_app_camera_csi_eof_count = 0U;
volatile uint32_t g_app_camera_vsync_count = 0U;
volatile uint32_t g_app_camera_limit_count = 0U;
volatile uint32_t g_app_camera_line_error_count = 0U;
volatile uint32_t g_app_camera_lane0_error_count = 0U;
volatile uint32_t g_app_camera_lane1_error_count = 0U;
volatile uint32_t g_app_camera_short_packet_count = 0U;
volatile uint32_t g_app_camera_error_count = 0U;
volatile uint32_t g_app_camera_last_error = 0U;
volatile uint32_t g_app_camera_hal_error = 0U;
volatile uint32_t g_app_camera_data_counter = 0U;
volatile uint32_t g_app_camera_phy_bitrate = APP_CAMERA_PRIMARY_PHY;
volatile uint32_t g_app_camera_fallback_count = 0U;
volatile uint32_t g_app_camera_frame0_addr = APP_CAMERA_FRAME0_ADDR;
volatile uint32_t g_app_camera_frame1_addr = APP_CAMERA_FRAME1_ADDR;
volatile uint32_t g_app_camera_frame_bytes = APP_CAMERA_CAPTURE_FRAME_BYTES;
volatile uint32_t g_app_camera_output_format = APP_CAMERA_CAPTURE_OUTPUT_FORMAT;
volatile uint32_t g_app_camera_line_pitch = APP_CAMERA_CAPTURE_LINE_BYTES;
volatile uint32_t g_app_camera_bayer_type = APP_CAMERA_RAW_BAYER_TYPE;
volatile uint32_t g_app_camera_test_pattern_enabled = 0U;
volatile uint32_t g_app_camera_completed_addr = APP_CAMERA_FRAME0_ADDR;
volatile uint32_t g_app_camera_debug_keepalive = 0U;
volatile uint32_t g_app_camera_csi_sr0 = 0U;
volatile uint32_t g_app_camera_csi_sr1 = 0U;
volatile uint32_t g_app_camera_csi_err1 = 0U;
volatile uint32_t g_app_camera_csi_err2 = 0U;
volatile uint32_t g_app_camera_csi_spdfr = 0U;
volatile uint32_t g_app_camera_csi_vc0cfgr1 = 0U;
volatile uint32_t g_app_camera_csi_vc0cfgr2 = 0U;
volatile uint32_t g_app_camera_csi_lmcfgr = 0U;
volatile uint32_t g_app_camera_csi_pcr = 0U;
volatile uint32_t g_app_camera_csi_prcr = 0U;
volatile uint32_t g_app_camera_csi_pmcr = 0U;
volatile uint32_t g_app_camera_csi_ier0 = 0U;
volatile uint32_t g_app_camera_csi_ier1 = 0U;
volatile uint32_t g_app_camera_csi_pfcr = 0U;
volatile uint32_t g_app_camera_dcmipp_cmier = 0U;
volatile uint32_t g_app_camera_dcmipp_cmsr1 = 0U;
volatile uint32_t g_app_camera_dcmipp_cmsr2 = 0U;
volatile uint32_t g_app_camera_dcmipp_p0fscr = 0U;
volatile uint32_t g_app_camera_dcmipp_p0fctcr = 0U;
volatile uint32_t g_app_camera_dcmipp_p0dccntr = 0U;
volatile uint32_t g_app_camera_dcmipp_p0dclmtr = 0U;
volatile uint32_t g_app_camera_dcmipp_p0ppcr = 0U;
volatile uint32_t g_app_camera_dcmipp_pipe_fscr = 0U;
volatile uint32_t g_app_camera_dcmipp_pipe_fctcr = 0U;
volatile uint32_t g_app_camera_dcmipp_pipe_ppcr = 0U;
volatile uint32_t g_app_camera_dcmipp_pipe_m0ar1 = 0U;
volatile uint32_t g_app_camera_dcmipp_pipe_m1ar1 = 0U;
volatile uint32_t g_app_camera_frame_sample_addr = 0U;
volatile uint32_t g_app_camera_frame_sample_min = 0U;
volatile uint32_t g_app_camera_frame_sample_max = 0U;
volatile uint32_t g_app_camera_frame_sample_avg = 0U;
volatile uint32_t g_app_camera_frame_sample_non_dark_count = 0U;
volatile uint32_t g_app_camera_frame_sample_change_count = 0U;
volatile uint32_t g_app_camera_frame_sample_center = 0U;
volatile uint32_t g_app_camera_frame_sample_seq = 0U;
/* AE/AWB servo inputs, refreshed per sampled frame (preview mode only). */
volatile uint32_t g_app_camera_frame_luma_avg = 0U;   /* 0..187 scale */
volatile uint32_t g_app_camera_frame_r_avg8 = 0U;     /* 0..255 */
volatile uint32_t g_app_camera_frame_g_avg8 = 0U;
volatile uint32_t g_app_camera_frame_b_avg8 = 0U;
/* AE/AWB servo state + diagnostics (SWD-visible). */
volatile uint32_t g_app_camera_ae_exposure_lines = 0x0D00U;
volatile uint32_t g_app_camera_ae_again_code = 0x80U;
volatile uint32_t g_app_camera_ae_dgain = 0x0100U;
volatile uint32_t g_app_camera_ae_update_count = 0U;
volatile uint32_t g_app_camera_wb_gain_r_q7 = 173U;   /* x1.35 */
volatile uint32_t g_app_camera_wb_gain_g_q7 = 128U;   /* x1.00 */
volatile uint32_t g_app_camera_wb_gain_b_q7 = 198U;   /* x1.55 */
volatile uint32_t g_app_camera_wb_update_count = 0U;

static void AppCamera_SnapshotRegisters(void)
{
  g_app_camera_csi_sr0 = CSI->SR0;
  g_app_camera_csi_sr1 = CSI->SR1;
  g_app_camera_csi_err1 = CSI->ERR1;
  g_app_camera_csi_err2 = CSI->ERR2;
  g_app_camera_csi_spdfr = CSI->SPDFR;
  g_app_camera_csi_vc0cfgr1 = CSI->VC0CFGR1;
  g_app_camera_csi_vc0cfgr2 = CSI->VC0CFGR2;
  g_app_camera_csi_lmcfgr = CSI->LMCFGR;
  g_app_camera_csi_pcr = CSI->PCR;
  g_app_camera_csi_prcr = CSI->PRCR;
  g_app_camera_csi_pmcr = CSI->PMCR;
  g_app_camera_csi_ier0 = CSI->IER0;
  g_app_camera_csi_ier1 = CSI->IER1;
  g_app_camera_csi_pfcr = CSI->PFCR;
  g_app_camera_dcmipp_cmier = DCMIPP->CMIER;
  g_app_camera_dcmipp_cmsr1 = DCMIPP->CMSR1;
  g_app_camera_dcmipp_cmsr2 = DCMIPP->CMSR2;
  g_app_camera_dcmipp_p0fscr = DCMIPP->P0FSCR;
  g_app_camera_dcmipp_p0fctcr = DCMIPP->P0FCTCR;
  g_app_camera_dcmipp_p0dccntr = DCMIPP->P0DCCNTR;
  g_app_camera_dcmipp_p0dclmtr = DCMIPP->P0DCLMTR;
  g_app_camera_dcmipp_p0ppcr = DCMIPP->P0PPCR;
  if (APP_CAMERA_DCMIPP_PIPE == DCMIPP_PIPE1)
  {
    g_app_camera_dcmipp_pipe_fscr = DCMIPP->P1FSCR;
    g_app_camera_dcmipp_pipe_fctcr = DCMIPP->P1FCTCR;
    g_app_camera_dcmipp_pipe_ppcr = DCMIPP->P1PPCR;
    g_app_camera_dcmipp_pipe_m0ar1 = DCMIPP->P1PPM0AR1;
    g_app_camera_dcmipp_pipe_m1ar1 = DCMIPP->P1PPM0AR2;
  }
  else
  {
    g_app_camera_dcmipp_pipe_fscr = DCMIPP->P0FSCR;
    g_app_camera_dcmipp_pipe_fctcr = DCMIPP->P0FCTCR;
    g_app_camera_dcmipp_pipe_ppcr = DCMIPP->P0PPCR;
    g_app_camera_dcmipp_pipe_m0ar1 = DCMIPP->P0PPM0AR1;
    g_app_camera_dcmipp_pipe_m1ar1 = DCMIPP->P0PPM0AR2;
  }
}

static void AppCamera_MirrorGlobals(void)
{
  g_app_camera_state = g_app_camera_status.flags;
  g_app_camera_chip_id = g_app_camera_status.chip_id;
  g_app_camera_frame_count = g_app_camera_status.frame_count;
  g_app_camera_csi_sof_count = g_app_camera_status.csi_sof_count;
  g_app_camera_csi_eof_count = g_app_camera_status.csi_eof_count;
  g_app_camera_vsync_count = g_app_camera_status.vsync_count;
  g_app_camera_limit_count = g_app_camera_status.limit_count;
  g_app_camera_line_error_count = g_app_camera_status.line_error_count;
  g_app_camera_lane0_error_count = g_app_camera_status.lane0_error_count;
  g_app_camera_lane1_error_count = g_app_camera_status.lane1_error_count;
  g_app_camera_short_packet_count = g_app_camera_status.short_packet_count;
  g_app_camera_error_count = g_app_camera_status.error_count;
  g_app_camera_last_error = (uint32_t)g_app_camera_status.last_error;
  g_app_camera_hal_error = g_app_camera_status.hal_error;
  g_app_camera_data_counter = g_app_camera_status.data_counter;
  g_app_camera_phy_bitrate = g_app_camera_status.phy_bitrate;
  g_app_camera_fallback_count = g_app_camera_status.fallback_count;
  g_app_camera_frame0_addr = g_app_camera_status.frame0_addr;
  g_app_camera_frame1_addr = g_app_camera_status.frame1_addr;
  g_app_camera_frame_bytes = g_app_camera_status.frame_bytes;
  g_app_camera_output_format = g_app_camera_status.output_format;
  g_app_camera_line_pitch = g_app_camera_status.line_pitch;
  g_app_camera_bayer_type = g_app_camera_status.bayer_type;
  g_app_camera_test_pattern_enabled = g_app_camera_status.test_pattern_enabled;
  g_app_camera_completed_addr = g_app_camera_status.completed_frame_addr;
  g_app_camera_status.display_addr = g_app_camera_display_addr;
  g_app_camera_status.pending_display_addr = g_app_camera_pending_display_addr;
  g_app_camera_status.ltdc_swap_count = g_app_camera_ltdc_swap_count;
  g_app_camera_status.ltdc_error_count = g_app_camera_ltdc_error_count;
  g_app_camera_status.ltdc_ier2 = g_app_camera_ltdc_ier2;
  g_app_camera_status.ltdc_isr2 = g_app_camera_ltdc_isr2;
  g_app_camera_status.ltdc_layer2_cr = g_app_camera_ltdc_layer2_cr;
  g_app_camera_status.ltdc_auto_disable_count = g_app_camera_ltdc_auto_disable_count;
  g_app_camera_debug_keepalive = (uint32_t)((uintptr_t)&AppCamera_Init ^
                                            (uintptr_t)&AppCamera_StartSmoke ^
                                            (uintptr_t)&AppCamera_StartPreview ^
                                            (uintptr_t)&AppCamera_Stop ^
                                            (uintptr_t)&AppCamera_SetTestPattern);
  if ((g_app_camera_status.flags & APP_CAMERA_FLAG_DCMIPP_READY) != 0U)
  {
    AppCamera_SnapshotRegisters();
  }
}

static void AppCamera_SetError(int32_t error)
{
  g_app_camera_status.last_error = error;
  g_app_camera_status.error_count++;
  g_app_camera_status.hal_error = HAL_DCMIPP_GetError(&hdcmipp);
  AppCamera_MirrorGlobals();
}

static void AppCamera_CacheCleanInvalidate(uint32_t address, uint32_t size)
{
  uint32_t aligned_addr = address & ~(APP_CAMERA_DCACHE_LINE_BYTES - 1U);
  uint32_t end_addr = (address + size + APP_CAMERA_DCACHE_LINE_BYTES - 1U) &
                      ~(APP_CAMERA_DCACHE_LINE_BYTES - 1U);

  SCB_CleanInvalidateDCache_by_Addr((void *)aligned_addr, (int32_t)(end_addr - aligned_addr));
}

static void AppCamera_CacheInvalidate(uint32_t address, uint32_t size)
{
  uint32_t aligned_addr = address & ~(APP_CAMERA_DCACHE_LINE_BYTES - 1U);
  uint32_t end_addr = (address + size + APP_CAMERA_DCACHE_LINE_BYTES - 1U) &
                      ~(APP_CAMERA_DCACHE_LINE_BYTES - 1U);

  if ((size == 0U) || ((SCB->CCR & SCB_CCR_DC_Msk) == 0U))
  {
    return;
  }

  SCB_InvalidateDCache_by_Addr((void *)aligned_addr, (int32_t)(end_addr - aligned_addr));
  __DSB();
  __ISB();
}

static uint32_t AppCamera_Rgb565Luma(uint32_t pixel)
{
  uint32_t r = (pixel >> 11) & 0x1FU;
  uint32_t g = (pixel >> 5) & 0x3FU;
  uint32_t b = pixel & 0x1FU;

  return (r * 2U) + g + (b * 2U);
}

static void AppCamera_SampleFrame(uint32_t frame_addr)
{
  uint32_t min_pixel = 0xffffU;
  uint32_t max_pixel = 0U;
  uint32_t sum_pixel = 0U;
  uint32_t non_dark_count = 0U;
  uint32_t change_count = 0U;
  uint32_t previous_pixel = 0xffffffffU;
  uint32_t center_pixel = 0U;
  uint32_t sample_count = 0U;
  uint32_t luma_sum = 0U;
  uint32_t r_sum = 0U;
  uint32_t g_sum = 0U;
  uint32_t b_sum = 0U;
  uint32_t line_pitch = g_app_camera_status.line_pitch;
  uint32_t output_bpp = g_app_camera_status.output_bpp;

  if ((frame_addr == 0U) || (line_pitch == 0U) || (output_bpp == 0U))
  {
    return;
  }

  for (uint32_t row = 0U; row < APP_CAMERA_FRAME_SAMPLE_GRID; row++)
  {
    uint32_t y = (row * (APP_CAMERA_HEIGHT - 1U)) / (APP_CAMERA_FRAME_SAMPLE_GRID - 1U);
    uint32_t row_addr = frame_addr + (y * line_pitch);

    AppCamera_CacheInvalidate(row_addr, line_pitch);

    for (uint32_t col = 0U; col < APP_CAMERA_FRAME_SAMPLE_GRID; col++)
    {
      uint32_t x = (col * (APP_CAMERA_WIDTH - 1U)) / (APP_CAMERA_FRAME_SAMPLE_GRID - 1U);
      uint32_t pixel;

      if (output_bpp == 2U)
      {
        pixel = ((const uint16_t *)(uintptr_t)row_addr)[x];
        if (AppCamera_Rgb565Luma(pixel) > APP_CAMERA_FRAME_DARK_LUMA)
        {
          non_dark_count++;
        }
        luma_sum += AppCamera_Rgb565Luma(pixel);
        r_sum += (pixel >> 11) & 0x1FU;
        g_sum += (pixel >> 5) & 0x3FU;
        b_sum += pixel & 0x1FU;
      }
      else
      {
        pixel = ((const uint8_t *)(uintptr_t)row_addr)[x];
        if (pixel > APP_CAMERA_FRAME_DARK_LUMA)
        {
          non_dark_count++;
        }
      }

      if (pixel < min_pixel)
      {
        min_pixel = pixel;
      }
      if (pixel > max_pixel)
      {
        max_pixel = pixel;
      }
      if ((previous_pixel != 0xffffffffU) && (pixel != previous_pixel))
      {
        change_count++;
      }
      previous_pixel = pixel;
      sum_pixel += pixel;
      sample_count++;

      if ((row == (APP_CAMERA_FRAME_SAMPLE_GRID / 2U)) &&
          (col == (APP_CAMERA_FRAME_SAMPLE_GRID / 2U)))
      {
        center_pixel = pixel;
      }
    }
  }

  if (sample_count == 0U)
  {
    return;
  }

  g_app_camera_frame_sample_addr = frame_addr;
  g_app_camera_frame_sample_min = min_pixel;
  g_app_camera_frame_sample_max = max_pixel;
  g_app_camera_frame_sample_avg = sum_pixel / sample_count;
  g_app_camera_frame_sample_non_dark_count = non_dark_count;
  g_app_camera_frame_sample_change_count = change_count;
  g_app_camera_frame_sample_center = center_pixel;
  if (output_bpp == 2U)
  {
    g_app_camera_frame_luma_avg = luma_sum / sample_count;
    g_app_camera_frame_r_avg8 = ((r_sum / sample_count) << 3) | ((r_sum / sample_count) >> 2);
    g_app_camera_frame_g_avg8 = ((g_sum / sample_count) << 2) | ((g_sum / sample_count) >> 4);
    g_app_camera_frame_b_avg8 = ((b_sum / sample_count) << 3) | ((b_sum / sample_count) >> 2);
  }
  g_app_camera_frame_sample_seq++;
}

static void AppCamera_CacheCleanInvalidatePreviewRows(void)
{
  AppCamera_CacheCleanInvalidate(APP_CAMERA_FRAME0_ADDR, APP_CAMERA_FRAME_BUFFER_BYTES);
  AppCamera_CacheCleanInvalidate(APP_CAMERA_FRAME1_ADDR, APP_CAMERA_FRAME_BUFFER_BYTES);
}

static void AppCamera_PrepareBuffers(uint8_t preview)
{
  if (preview != 0U)
  {
    AppCamera_CacheCleanInvalidatePreviewRows();
  }
  else
  {
    (void)memset((void *)APP_CAMERA_FRAME0_ADDR, 0, APP_CAMERA_FRAME_BUFFER_BYTES);
    (void)memset((void *)APP_CAMERA_FRAME1_ADDR, 0, APP_CAMERA_FRAME_BUFFER_BYTES);
    AppCamera_CacheCleanInvalidate(APP_CAMERA_FRAME0_ADDR, APP_CAMERA_FRAME_BUFFER_BYTES);
    AppCamera_CacheCleanInvalidate(APP_CAMERA_FRAME1_ADDR, APP_CAMERA_FRAME_BUFFER_BYTES);
  }
}

static void AppCamera_SetOutputMode(uint8_t preview)
{
  if (preview != 0U)
  {
    g_app_camera_status.frame0_addr = APP_CAMERA_FRAME0_ADDR;
    g_app_camera_status.frame1_addr = APP_CAMERA_FRAME1_ADDR;
    g_app_camera_status.output_format = APP_CAMERA_PREVIEW_OUTPUT_FORMAT;
    g_app_camera_status.output_bpp = APP_CAMERA_PREVIEW_OUTPUT_BPP;
    g_app_camera_status.line_pitch = APP_CAMERA_PREVIEW_LINE_BYTES;
    g_app_camera_status.frame_bytes = APP_CAMERA_PREVIEW_FRAME_BYTES;
    g_app_camera_status.completed_frame_addr = APP_CAMERA_FRAME0_ADDR;
  }
  else
  {
    g_app_camera_status.frame0_addr = APP_CAMERA_FRAME0_ADDR;
    g_app_camera_status.frame1_addr = APP_CAMERA_FRAME1_ADDR;
    g_app_camera_status.output_format = APP_CAMERA_CAPTURE_OUTPUT_FORMAT;
    g_app_camera_status.output_bpp = APP_CAMERA_CAPTURE_OUTPUT_BPP;
    g_app_camera_status.line_pitch = APP_CAMERA_CAPTURE_LINE_BYTES;
    g_app_camera_status.frame_bytes = APP_CAMERA_CAPTURE_FRAME_BYTES;
    g_app_camera_status.completed_frame_addr = APP_CAMERA_FRAME0_ADDR;
  }
  g_app_camera_status.bayer_type = APP_CAMERA_RAW_BAYER_TYPE;
  g_app_camera_status.frame_buffer_bytes = APP_CAMERA_FRAME_BUFFER_BYTES;
}

/* Q7 gain (128 = x1.0) to the DCMIPP exposure block's shift+multiplier
 * encoding (gain = multiplier * 2^shift / 128), mirroring ST's ISP
 * library conversion. */
static void AppCamera_GainToShiftMult(uint32_t gain_q7, uint8_t *shift, uint8_t *mult)
{
  uint32_t value = gain_q7;
  uint8_t shift_count = 0U;

  while ((value >= 256U) && (shift_count < 7U))
  {
    value >>= 1;
    shift_count++;
  }
  if (value > 255U)
  {
    value = 255U;
  }

  *shift = shift_count;
  *mult = (uint8_t)value;
}

/* Push the current white-balance gains into the pre-demosaic exposure
 * block. Safe while streaming (shadow registers load per frame). */
static int32_t AppCamera_ApplyWhiteBalance(void)
{
  DCMIPP_ExposureConfTypeDef exposure_config = {0};
  HAL_StatusTypeDef hal_status;

  AppCamera_GainToShiftMult(g_app_camera_wb_gain_r_q7,
                            &exposure_config.ShiftRed, &exposure_config.MultiplierRed);
  AppCamera_GainToShiftMult(g_app_camera_wb_gain_g_q7,
                            &exposure_config.ShiftGreen, &exposure_config.MultiplierGreen);
  AppCamera_GainToShiftMult(g_app_camera_wb_gain_b_q7,
                            &exposure_config.ShiftBlue, &exposure_config.MultiplierBlue);

  hal_status = HAL_DCMIPP_PIPE_SetISPExposureConfig(&hdcmipp, APP_CAMERA_DCMIPP_PIPE,
                                                    &exposure_config);
  if (hal_status == HAL_OK)
  {
    hal_status = HAL_DCMIPP_PIPE_EnableISPExposure(&hdcmipp, APP_CAMERA_DCMIPP_PIPE);
  }

  return (hal_status == HAL_OK) ? APP_CAMERA_OK : APP_CAMERA_ERROR_DCMIPP_CONFIG;
}

static int32_t AppCamera_ConfigureDcmipp(uint32_t phy_bitrate, uint8_t preview)
{
  DCMIPP_CSI_ConfTypeDef csi_config = {0};
  DCMIPP_CSI_VCFilteringConfTypeDef vc_filter_config = {0};
  DCMIPP_CSI_PIPE_ConfTypeDef csi_pipe_config = {0};
  DCMIPP_PipeConfTypeDef pipe_config = {0};
  DCMIPP_RawBayer2RGBConfTypeDef raw_bayer_config = {0};
  HAL_StatusTypeDef hal_status;

  hdcmipp.Instance = DCMIPP;

  hal_status = HAL_DCMIPP_Init(&hdcmipp);
  if (hal_status != HAL_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_INIT);
    return APP_CAMERA_ERROR_DCMIPP_INIT;
  }

  csi_config.NumberOfLanes = DCMIPP_CSI_TWO_DATA_LANES;
  csi_config.DataLaneMapping = APP_CAMERA_DATA_LANE_MAPPING;
  csi_config.PHYBitrate = phy_bitrate;
  hal_status = HAL_DCMIPP_CSI_SetConfig(&hdcmipp, &csi_config);
  if (hal_status != HAL_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
    return APP_CAMERA_ERROR_DCMIPP_CONFIG;
  }

  vc_filter_config.DataTypeNB = 1U;
  vc_filter_config.DataTypeClass[0] = DCMIPP_DT_RAW10;
  vc_filter_config.DataTypeFormat[0] = DCMIPP_CSI_DT_BPP10;
  hal_status = HAL_DCMIPP_CSI_SetVCFilteringConfig(&hdcmipp, APP_CAMERA_DCMIPP_VC, &vc_filter_config);
  if (hal_status != HAL_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
    return APP_CAMERA_ERROR_DCMIPP_CONFIG;
  }

  hal_status = HAL_DCMIPP_CSI_SetVCConfig(&hdcmipp, APP_CAMERA_DCMIPP_VC, DCMIPP_CSI_DT_BPP10);
  if (hal_status != HAL_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
    return APP_CAMERA_ERROR_DCMIPP_CONFIG;
  }

  csi_pipe_config.DataTypeMode = DCMIPP_DTMODE_DTIDA;
  csi_pipe_config.DataTypeIDA = DCMIPP_DT_RAW10;
  csi_pipe_config.DataTypeIDB = DCMIPP_DT_RAW10;
  hal_status = HAL_DCMIPP_CSI_PIPE_SetConfig(&hdcmipp, APP_CAMERA_DCMIPP_PIPE, &csi_pipe_config);
  if (hal_status != HAL_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
    return APP_CAMERA_ERROR_DCMIPP_CONFIG;
  }

  pipe_config.FrameRate = DCMIPP_FRAME_RATE_ALL;
  pipe_config.PixelPipePitch = g_app_camera_status.line_pitch;
  pipe_config.PixelPackerFormat = g_app_camera_status.output_format;
  hal_status = HAL_DCMIPP_PIPE_SetConfig(&hdcmipp, APP_CAMERA_DCMIPP_PIPE, &pipe_config);
  if (hal_status != HAL_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
    return APP_CAMERA_ERROR_DCMIPP_CONFIG;
  }

  if (preview != 0U)
  {
    raw_bayer_config.RawBayerType = APP_CAMERA_RAW_BAYER_TYPE;
    raw_bayer_config.PeakStrength = APP_CAMERA_RAW_BAYER_STRENGTH;
    raw_bayer_config.VLineStrength = APP_CAMERA_RAW_BAYER_STRENGTH;
    raw_bayer_config.HLineStrength = APP_CAMERA_RAW_BAYER_STRENGTH;
    raw_bayer_config.EdgeStrength = APP_CAMERA_RAW_BAYER_STRENGTH;
    hal_status = HAL_DCMIPP_PIPE_SetISPRawBayer2RGBConfig(&hdcmipp,
                                                          APP_CAMERA_DCMIPP_PIPE,
                                                          &raw_bayer_config);
    if (hal_status != HAL_OK)
    {
      AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
      return APP_CAMERA_ERROR_DCMIPP_CONFIG;
    }

    hal_status = HAL_DCMIPP_PIPE_EnableISPRawBayer2RGB(&hdcmipp, APP_CAMERA_DCMIPP_PIPE);
    if (hal_status != HAL_OK)
    {
      AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
      return APP_CAMERA_ERROR_DCMIPP_CONFIG;
    }

    /* Minimal ISP chain (gray/washed-out fix): black-level subtraction on
     * the raw pedestal, per-channel white-balance gains pre-demosaic, and
     * the hardware gamma curve on the RGB output. Order in hardware is
     * fixed by the pipe; we only configure and enable the blocks. */
    {
      DCMIPP_BlackLevelConfTypeDef black_level_config;

      black_level_config.RedCompBlackLevel = (uint8_t)APP_CAMERA_ISP_BLACK_LEVEL;
      black_level_config.GreenCompBlackLevel = (uint8_t)APP_CAMERA_ISP_BLACK_LEVEL;
      black_level_config.BlueCompBlackLevel = (uint8_t)APP_CAMERA_ISP_BLACK_LEVEL;
      hal_status = HAL_DCMIPP_PIPE_SetISPBlackLevelCalibrationConfig(&hdcmipp,
                                                                     APP_CAMERA_DCMIPP_PIPE,
                                                                     &black_level_config);
      if (hal_status == HAL_OK)
      {
        hal_status = HAL_DCMIPP_PIPE_EnableISPBlackLevelCalibration(&hdcmipp,
                                                                    APP_CAMERA_DCMIPP_PIPE);
      }
      if (hal_status != HAL_OK)
      {
        AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
        return APP_CAMERA_ERROR_DCMIPP_CONFIG;
      }

      if (AppCamera_ApplyWhiteBalance() != APP_CAMERA_OK)
      {
        AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
        return APP_CAMERA_ERROR_DCMIPP_CONFIG;
      }

      hal_status = HAL_DCMIPP_PIPE_EnableGammaConversion(&hdcmipp, APP_CAMERA_DCMIPP_PIPE);
      if (hal_status != HAL_OK)
      {
        AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
        return APP_CAMERA_ERROR_DCMIPP_CONFIG;
      }
    }
  }

  if (APP_CAMERA_DCMIPP_PIPE == DCMIPP_PIPE0)
  {
    hal_status = HAL_DCMIPP_PIPE_EnableLimitEvent(&hdcmipp, APP_CAMERA_DCMIPP_PIPE, APP_CAMERA_RAW10_FRAME_WORDS);
    if (hal_status != HAL_OK)
    {
      AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
      return APP_CAMERA_ERROR_DCMIPP_CONFIG;
    }
  }

  g_app_camera_status.flags |= APP_CAMERA_FLAG_DCMIPP_READY;
  g_app_camera_status.phy_bitrate = phy_bitrate;
  AppCamera_MirrorGlobals();

  return APP_CAMERA_OK;
}

static int32_t AppCamera_StartDcmippCapture(uint8_t preview)
{
  HAL_StatusTypeDef hal_status;

  (void)preview;

  hal_status = HAL_DCMIPP_CSI_PIPE_DoubleBufferStart(&hdcmipp,
                                                     APP_CAMERA_DCMIPP_PIPE,
                                                     APP_CAMERA_DCMIPP_VC,
                                                     APP_CAMERA_FRAME0_ADDR,
                                                     APP_CAMERA_FRAME1_ADDR,
                                                     DCMIPP_MODE_CONTINUOUS);
  if (hal_status != HAL_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_START);
    return APP_CAMERA_ERROR_DCMIPP_START;
  }

  return APP_CAMERA_OK;
}

static int32_t AppCamera_RestartWithPhy(uint32_t phy_bitrate)
{
  int32_t status;

  g_app_camera_status.fallback_count++;
  g_app_camera_status.flags &= ~(APP_CAMERA_FLAG_DCMIPP_READY | APP_CAMERA_FLAG_STREAMING);
  if (phy_bitrate != APP_CAMERA_PRIMARY_PHY)
  {
    g_app_camera_status.flags |= APP_CAMERA_FLAG_FALLBACK_PHY;
  }
  else
  {
    g_app_camera_status.flags &= ~APP_CAMERA_FLAG_FALLBACK_PHY;
  }

  (void)AppCameraIMX219_SetStream(0U);
  (void)HAL_DCMIPP_CSI_PIPE_Stop(&hdcmipp, APP_CAMERA_DCMIPP_PIPE, APP_CAMERA_DCMIPP_VC);
  (void)HAL_DCMIPP_DeInit(&hdcmipp);

  status = AppCamera_ConfigureDcmipp(phy_bitrate,
                                     ((g_app_camera_status.flags & APP_CAMERA_FLAG_PREVIEW) != 0U) ? 1U : 0U);
  if (status != APP_CAMERA_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_FALLBACK_RESTART);
    return status;
  }

  status = AppCamera_StartDcmippCapture(((g_app_camera_status.flags & APP_CAMERA_FLAG_PREVIEW) != 0U) ? 1U : 0U);
  if (status != APP_CAMERA_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_FALLBACK_RESTART);
    return status;
  }

  status = AppCameraIMX219_SetStream(1U);
  if (status != APP_CAMERA_IMX219_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_SENSOR_STREAM);
    return APP_CAMERA_ERROR_SENSOR_STREAM;
  }

  g_app_camera_status.flags |= APP_CAMERA_FLAG_STREAMING;
  AppCamera_MirrorGlobals();

  return APP_CAMERA_OK;
}

int32_t AppCamera_Init(void)
{
  AppCameraIMX219Config_t sensor_config;
  uint16_t chip_id = 0U;
  int32_t status;

  g_app_camera_status.flags = 0U;
  g_app_camera_status.chip_id = 0U;
  g_app_camera_status.width = APP_CAMERA_WIDTH;
  g_app_camera_status.height = APP_CAMERA_HEIGHT;
  g_app_camera_status.fps = APP_CAMERA_FPS;
  g_app_camera_status.frame_count = 0U;
  g_app_camera_status.csi_sof_count = 0U;
  g_app_camera_status.csi_eof_count = 0U;
  g_app_camera_status.vsync_count = 0U;
  g_app_camera_status.limit_count = 0U;
  g_app_camera_status.line_error_count = 0U;
  g_app_camera_status.lane0_error_count = 0U;
  g_app_camera_status.lane1_error_count = 0U;
  g_app_camera_status.short_packet_count = 0U;
  g_app_camera_status.error_count = 0U;
  g_app_camera_status.last_error = APP_CAMERA_OK;
  g_app_camera_status.hal_error = 0U;
  g_app_camera_status.data_counter = 0U;
  g_app_camera_status.frame0_addr = APP_CAMERA_FRAME0_ADDR;
  g_app_camera_status.frame1_addr = APP_CAMERA_FRAME1_ADDR;
  AppCamera_SetOutputMode(0U);
  g_app_camera_status.test_pattern_enabled = 0U;
  g_app_camera_status.completed_frame_addr = APP_CAMERA_FRAME0_ADDR;
  g_app_camera_status.display_addr = g_app_camera_display_addr;
  g_app_camera_status.pending_display_addr = g_app_camera_pending_display_addr;
  g_app_camera_status.ltdc_swap_count = g_app_camera_ltdc_swap_count;
  g_app_camera_status.ltdc_error_count = g_app_camera_ltdc_error_count;
  g_app_camera_status.ltdc_ier2 = g_app_camera_ltdc_ier2;
  g_app_camera_status.ltdc_isr2 = g_app_camera_ltdc_isr2;
  g_app_camera_status.ltdc_layer2_cr = g_app_camera_ltdc_layer2_cr;
  g_app_camera_status.ltdc_auto_disable_count = g_app_camera_ltdc_auto_disable_count;
  g_app_camera_status.pipe_index = APP_CAMERA_DCMIPP_PIPE;
  g_app_camera_status.phy_bitrate = APP_CAMERA_PRIMARY_PHY;
  g_app_camera_status.fallback_count = 0U;
  g_app_camera_status.poll_count = 0U;
  g_app_camera_last_frame_poll = 0U;
  g_app_camera_no_frame_ms = 0U;
  g_app_camera_phy_retry_index = 0U;
  g_app_camera_stream_recover_attempts = 0U;

  sensor_config.width = APP_CAMERA_WIDTH;
  sensor_config.height = APP_CAMERA_HEIGHT;
  sensor_config.fps = APP_CAMERA_FPS;
  sensor_config.input_clock_hz = APP_CAMERA_INPUT_CLOCK_HZ;

  status = AppCameraIMX219_Init(&sensor_config, &chip_id);
  g_app_camera_status.chip_id = chip_id;
  g_app_camera_status.flags |= APP_CAMERA_FLAG_POWERED;

  if (status != APP_CAMERA_IMX219_OK)
  {
    g_app_camera_init_status = (uint32_t)status;
    AppCamera_SetError(APP_CAMERA_ERROR_SENSOR_INIT);
    return APP_CAMERA_ERROR_SENSOR_INIT;
  }

  g_app_camera_status.flags |= APP_CAMERA_FLAG_I2C_OK |
                               APP_CAMERA_FLAG_CHIP_ID_OK |
                               APP_CAMERA_FLAG_SENSOR_CONFIGURED;
  g_app_camera_init_status = APP_CAMERA_OK;
  AppCamera_MirrorGlobals();

  return APP_CAMERA_OK;
}

static int32_t AppCamera_StartCapture(uint8_t preview)
{
  int32_t status;

  if ((g_app_camera_status.flags & APP_CAMERA_FLAG_SENSOR_CONFIGURED) == 0U)
  {
    status = AppCamera_Init();
    if (status != APP_CAMERA_OK)
    {
      g_app_camera_start_status = (uint32_t)status;
      return status;
    }
  }

  if (((g_app_camera_status.flags & APP_CAMERA_FLAG_STREAMING) != 0U) &&
      ((((g_app_camera_status.flags & APP_CAMERA_FLAG_PREVIEW) != 0U) ? 1U : 0U) != preview))
  {
    (void)AppCamera_Stop();
  }

  if ((g_app_camera_status.flags & APP_CAMERA_FLAG_STREAMING) != 0U)
  {
    if (preview != 0U)
    {
      status = AppCameraDisplay_InitLayers(APP_CAMERA_DISPLAY_TARGET_ADDR);
      if (status != APP_CAMERA_DISPLAY_OK)
      {
        AppCamera_SetError(APP_CAMERA_ERROR_DISPLAY_INIT);
        g_app_camera_start_status = APP_CAMERA_ERROR_DISPLAY_INIT;
        return APP_CAMERA_ERROR_DISPLAY_INIT;
      }
      /* Layer visibility is owned by the UI Model (per-page policy). */
      g_app_camera_status.flags |= APP_CAMERA_FLAG_PREVIEW | APP_CAMERA_FLAG_DISPLAY_READY;
    }
    g_app_camera_start_status = APP_CAMERA_OK;
    AppCamera_MirrorGlobals();
    return APP_CAMERA_OK;
  }

  AppCamera_SetOutputMode(preview);
  AppCamera_PrepareBuffers(preview);

  if (preview != 0U)
  {
    status = AppCameraDisplay_InitLayers(APP_CAMERA_DISPLAY_TARGET_ADDR);
    if (status != APP_CAMERA_DISPLAY_OK)
    {
      AppCamera_SetError(APP_CAMERA_ERROR_DISPLAY_INIT);
      g_app_camera_start_status = APP_CAMERA_ERROR_DISPLAY_INIT;
      return APP_CAMERA_ERROR_DISPLAY_INIT;
    }
    /* Layer visibility is owned by the UI Model (per-page policy). */
    g_app_camera_status.flags |= APP_CAMERA_FLAG_PREVIEW | APP_CAMERA_FLAG_DISPLAY_READY;
  }
  else
  {
    g_app_camera_status.flags &= ~(APP_CAMERA_FLAG_PREVIEW | APP_CAMERA_FLAG_DISPLAY_READY);
  }

  status = AppCamera_ConfigureDcmipp(APP_CAMERA_PRIMARY_PHY, preview);
  if (status != APP_CAMERA_OK)
  {
    g_app_camera_start_status = (uint32_t)status;
    return status;
  }

  status = AppCamera_StartDcmippCapture(preview);
  if (status != APP_CAMERA_OK)
  {
    g_app_camera_start_status = (uint32_t)status;
    return status;
  }

  status = AppCameraIMX219_SetStream(1U);
  if (status != APP_CAMERA_IMX219_OK)
  {
    (void)HAL_DCMIPP_CSI_PIPE_Stop(&hdcmipp, APP_CAMERA_DCMIPP_PIPE, APP_CAMERA_DCMIPP_VC);
    AppCamera_SetError(APP_CAMERA_ERROR_SENSOR_STREAM);
    g_app_camera_start_status = APP_CAMERA_ERROR_SENSOR_STREAM;
    return APP_CAMERA_ERROR_SENSOR_STREAM;
  }

  g_app_camera_status.flags |= APP_CAMERA_FLAG_STREAMING;
  g_app_camera_start_status = APP_CAMERA_OK;
  AppCamera_MirrorGlobals();

  return APP_CAMERA_OK;
}

int32_t AppCamera_StartSmoke(void)
{
  return AppCamera_StartCapture(0U);
}

int32_t AppCamera_StartPreview(void)
{
  return AppCamera_StartCapture(1U);
}

int32_t AppCamera_Stop(void)
{
  if ((g_app_camera_status.flags & APP_CAMERA_FLAG_STREAMING) != 0U)
  {
    (void)AppCameraIMX219_SetStream(0U);
    (void)HAL_DCMIPP_CSI_PIPE_Stop(&hdcmipp, APP_CAMERA_DCMIPP_PIPE, APP_CAMERA_DCMIPP_VC);
  }
  if ((g_app_camera_status.flags & APP_CAMERA_FLAG_DCMIPP_READY) != 0U)
  {
    (void)HAL_DCMIPP_DeInit(&hdcmipp);
  }
  AppCameraDisplay_SetVisible(0U);

  g_app_camera_status.flags &= ~(APP_CAMERA_FLAG_STREAMING |
                                 APP_CAMERA_FLAG_DCMIPP_READY |
                                 APP_CAMERA_FLAG_FRAME_SEEN |
                                 APP_CAMERA_FLAG_PREVIEW |
                                 APP_CAMERA_FLAG_DISPLAY_READY);
  g_app_camera_start_status = APP_CAMERA_OK;
  AppCamera_MirrorGlobals();

  return APP_CAMERA_OK;
}

int32_t AppCamera_SetTestPattern(uint8_t enable)
{
  int32_t status;

  if ((g_app_camera_status.flags & APP_CAMERA_FLAG_SENSOR_CONFIGURED) == 0U)
  {
    status = AppCamera_Init();
    if (status != APP_CAMERA_OK)
    {
      return status;
    }
  }

  status = AppCameraIMX219_SetTestPattern(enable);
  if (status != APP_CAMERA_IMX219_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_TEST_PATTERN);
    return APP_CAMERA_ERROR_TEST_PATTERN;
  }

  g_app_camera_status.test_pattern_enabled = (enable != 0U) ? 1U : 0U;
  if (enable != 0U)
  {
    g_app_camera_status.flags |= APP_CAMERA_FLAG_TEST_PATTERN;
  }
  else
  {
    g_app_camera_status.flags &= ~APP_CAMERA_FLAG_TEST_PATTERN;
  }
  AppCamera_MirrorGlobals();

  return APP_CAMERA_OK;
}

/* 1 Hz auto-exposure: multiplicative servo on average luma with a
 * gain ladder (integration time first, then analog gain, then digital
 * gain). Runs in the bring-up thread, so the I2C writes are legal. */
static void AppCamera_RunAutoExposure(uint32_t luma_avg)
{
  uint32_t ratio_q8;
  uint64_t total;
  uint32_t exposure;
  uint32_t again_x256;
  uint32_t dgain;

  if (luma_avg < 1U)
  {
    luma_avg = 1U;
  }
  if ((luma_avg + APP_CAMERA_AE_DEADBAND >= APP_CAMERA_AE_TARGET_LUMA) &&
      (luma_avg <= APP_CAMERA_AE_TARGET_LUMA + APP_CAMERA_AE_DEADBAND))
  {
    return;
  }

  /* Per-step correction ratio, clamped to +/-35% to converge without
   * oscillating on the 1 Hz loop. */
  ratio_q8 = (APP_CAMERA_AE_TARGET_LUMA * 256U) / luma_avg;
  if (ratio_q8 > 345U)
  {
    ratio_q8 = 345U;
  }
  if (ratio_q8 < 166U)
  {
    ratio_q8 = 166U;
  }

  /* Current total gain expressed as exposure * analog * digital (Q8 each
   * for the gain terms). Analog gain code c gives gain 256/(256-c). */
  again_x256 = (256U * 256U) / (256U - g_app_camera_ae_again_code);
  dgain = g_app_camera_ae_dgain;
  total = (uint64_t)g_app_camera_ae_exposure_lines * again_x256 * dgain;
  total = (total * ratio_q8) >> 8;

  /* Redistribute along the ladder: exposure first (no noise cost), then
   * analog gain, then digital gain. */
  exposure = (uint32_t)(total / ((uint64_t)256U * 256U));
  again_x256 = 256U;
  dgain = 256U;
  if (exposure > APP_CAMERA_AE_MAX_EXPOSURE)
  {
    uint64_t remainder = total / APP_CAMERA_AE_MAX_EXPOSURE;

    exposure = APP_CAMERA_AE_MAX_EXPOSURE;
    again_x256 = (uint32_t)(remainder / 256U);
    if (again_x256 < 256U)
    {
      again_x256 = 256U;
    }
    {
      const uint32_t max_again_x256 =
          (256U * 256U) / (256U - APP_CAMERA_AE_MAX_AGAIN_CODE);

      if (again_x256 > max_again_x256)
      {
        /* remainder is Q16 (analog x digital); dividing by the Q8 analog
         * cap leaves the Q8 digital gain. */
        dgain = (uint32_t)(remainder / max_again_x256);
        again_x256 = max_again_x256;
        if (dgain < APP_CAMERA_AE_MIN_DGAIN)
        {
          dgain = APP_CAMERA_AE_MIN_DGAIN;
        }
        if (dgain > APP_CAMERA_AE_MAX_DGAIN)
        {
          dgain = APP_CAMERA_AE_MAX_DGAIN;
        }
      }
    }
  }
  else if (exposure < APP_CAMERA_AE_MIN_EXPOSURE)
  {
    exposure = APP_CAMERA_AE_MIN_EXPOSURE;
  }

  {
    /* Convert analog gain factor back to the IMX219 code c = 256 - 256/g. */
    uint32_t again_code = 256U - ((256U * 256U) / again_x256);

    if (again_code > APP_CAMERA_AE_MAX_AGAIN_CODE)
    {
      again_code = APP_CAMERA_AE_MAX_AGAIN_CODE;
    }

    if ((exposure == g_app_camera_ae_exposure_lines) &&
        (again_code == g_app_camera_ae_again_code) &&
        (dgain == g_app_camera_ae_dgain))
    {
      return;
    }

    if (AppCameraIMX219_SetExposure((uint16_t)exposure,
                                    (uint8_t)again_code,
                                    (uint16_t)dgain) == APP_CAMERA_IMX219_OK)
    {
      g_app_camera_ae_exposure_lines = exposure;
      g_app_camera_ae_again_code = again_code;
      g_app_camera_ae_dgain = dgain;
      g_app_camera_ae_update_count++;
    }
  }
}

/* 1 Hz grey-world white balance on the pre-demosaic channel gains.
 * Register-only writes, no I2C. */
static void AppCamera_RunAutoWhiteBalance(uint32_t r_avg8, uint32_t g_avg8, uint32_t b_avg8)
{
  uint32_t target_r;
  uint32_t target_b;
  uint32_t new_r;
  uint32_t new_b;

  if ((r_avg8 < 4U) || (g_avg8 < 4U) || (b_avg8 < 4U))
  {
    return; /* Too dark for a meaningful ratio. */
  }

  /* Ideal gains that would equalise the channel means, then IIR-step a
   * quarter of the way there per update. */
  target_r = (g_app_camera_wb_gain_r_q7 * g_avg8) / r_avg8;
  target_b = (g_app_camera_wb_gain_b_q7 * g_avg8) / b_avg8;
  new_r = g_app_camera_wb_gain_r_q7 + ((int32_t)(target_r - g_app_camera_wb_gain_r_q7) / 4);
  new_b = g_app_camera_wb_gain_b_q7 + ((int32_t)(target_b - g_app_camera_wb_gain_b_q7) / 4);

  if (new_r < APP_CAMERA_WB_GAIN_MIN)
  {
    new_r = APP_CAMERA_WB_GAIN_MIN;
  }
  if (new_r > APP_CAMERA_WB_GAIN_MAX)
  {
    new_r = APP_CAMERA_WB_GAIN_MAX;
  }
  if (new_b < APP_CAMERA_WB_GAIN_MIN)
  {
    new_b = APP_CAMERA_WB_GAIN_MIN;
  }
  if (new_b > APP_CAMERA_WB_GAIN_MAX)
  {
    new_b = APP_CAMERA_WB_GAIN_MAX;
  }

  /* Deadband: skip register churn for sub-2% moves. */
  if (((new_r > g_app_camera_wb_gain_r_q7) ? (new_r - g_app_camera_wb_gain_r_q7)
                                           : (g_app_camera_wb_gain_r_q7 - new_r)) < 3U &&
      ((new_b > g_app_camera_wb_gain_b_q7) ? (new_b - g_app_camera_wb_gain_b_q7)
                                           : (g_app_camera_wb_gain_b_q7 - new_b)) < 3U)
  {
    return;
  }

  g_app_camera_wb_gain_r_q7 = new_r;
  g_app_camera_wb_gain_b_q7 = new_b;
  if (AppCamera_ApplyWhiteBalance() == APP_CAMERA_OK)
  {
    g_app_camera_wb_update_count++;
  }
}

/* GDB screenshot hook: while nonzero, the 1 Hz poll stops the sensor stream
 * and the DCMIPP pipe, so the displayed buffer (including the drawn acoustic
 * overlay) survives a debugger halt + memory dump instead of being ping-pong
 * overwritten by capture DMA. Cleared -> capture restarts transparently. */
volatile uint32_t g_app_camera_freeze_request = 0U;
static volatile uint8_t s_camera_frozen = 0U;

/* Called by the display swap worker right after an overlay pass. Parks the
 * pipe only when this frame actually carries the acoustic overlay
 * (overlay_drawn), so the frozen frame deterministically shows the heatmap
 * instead of racing a momentary quality-gate dropout. With no detection at
 * all the freeze simply never latches and the capture proceeds live. */
void AppCamera_FreezeIfRequested(uint8_t overlay_drawn)
{
  if ((g_app_camera_freeze_request != 0U) && (s_camera_frozen == 0U) &&
      (overlay_drawn != 0U) &&
      ((g_app_camera_status.flags & APP_CAMERA_FLAG_STREAMING) != 0U))
  {
    /* Sensor stream off ONLY - the DCMIPP pipe stays armed and simply
     * starves. Aborting the pipe mid-frame (PIPE_Stop) leaves a partially
     * written buffer that shows up as banding artifacts in the dump. Any
     * in-flight frame completes normally, then the display goes quiet. */
    (void)AppCameraIMX219_SetStream(0U);
    s_camera_frozen = 1U;
  }
}

void AppCamera_Poll(uint32_t elapsed_ms)
{
  uint32_t data_counter = 0U;

  g_app_camera_status.poll_count++;

  if (g_app_camera_freeze_request != 0U)
  {
    /* Freeze pending or applied (the swap worker parks the pipe): skip the
     * stall watchdog and stream-recovery paths below, they would restart
     * the pipe we are trying to park. */
    return;
  }
  if (s_camera_frozen != 0U)
  {
    /* Pipe was never stopped (sensor-only freeze): re-enabling the stream
     * resumes the continuous double-buffer capture as-is. */
    s_camera_frozen = 0U;
    (void)AppCameraIMX219_SetStream(1U);
    g_app_camera_no_frame_ms = 0U;
  }

  if ((g_app_camera_status.flags & APP_CAMERA_FLAG_DCMIPP_READY) != 0U)
  {
    if (HAL_DCMIPP_PIPE_GetDataCounter(&hdcmipp, APP_CAMERA_DCMIPP_PIPE, &data_counter) == HAL_OK)
    {
      g_app_camera_status.data_counter = data_counter;
    }
    g_app_camera_status.hal_error = HAL_DCMIPP_GetError(&hdcmipp);
  }

  if (g_app_camera_status.frame_count != g_app_camera_last_frame_poll)
  {
    g_app_camera_last_frame_poll = g_app_camera_status.frame_count;
    g_app_camera_no_frame_ms = 0U;
    /* Frames are flowing again: re-arm the stream recovery budget. */
    g_app_camera_stream_recover_attempts = 0U;
  }
  else if ((g_app_camera_status.flags & APP_CAMERA_FLAG_STREAMING) != 0U)
  {
    g_app_camera_no_frame_ms += elapsed_ms;
  }
  else
  {
    g_app_camera_no_frame_ms = 0U;
  }

  if (((g_app_camera_status.flags & APP_CAMERA_FLAG_STREAMING) != 0U) &&
      (g_app_camera_status.frame_count == 0U) &&
      (g_app_camera_status.line_error_count != 0U) &&
      ((g_app_camera_phy_retry_index + 1U) <
       (sizeof(g_app_camera_phy_retry_list) / sizeof(g_app_camera_phy_retry_list[0]))) &&
      (g_app_camera_no_frame_ms >= APP_CAMERA_DPHY_RETRY_DELAY_MS))
  {
    g_app_camera_phy_retry_index++;
    (void)AppCamera_RestartWithPhy(g_app_camera_phy_retry_list[g_app_camera_phy_retry_index]);
    g_app_camera_no_frame_ms = 0U;
  }

  /* A restart can fail transiently when another owner (PCMD config window,
   * touch) holds the I2C2 lock for longer than the sensor-write timeout; the
   * restart path stops the stream before the failing write, which would
   * otherwise leave the camera dead. Re-attempt on the 1 Hz poll with a
   * bounded budget instead of giving up permanently. */
  if (((g_app_camera_status.flags & APP_CAMERA_FLAG_SENSOR_CONFIGURED) != 0U) &&
      ((g_app_camera_status.flags & APP_CAMERA_FLAG_STREAMING) == 0U) &&
      ((g_app_camera_status.flags & APP_CAMERA_FLAG_PREVIEW) != 0U) &&
      (g_app_camera_stream_recover_attempts < 8U))
  {
    /* Budget resets only when frames actually flow (above), so a restart
     * that "succeeds" without producing frames cannot loop forever. */
    g_app_camera_stream_recover_attempts++;
    (void)AppCamera_RestartWithPhy(g_app_camera_phy_retry_list[g_app_camera_phy_retry_index]);
  }

  /* AE/AWB servos: preview only, and only when frames are flowing so the
   * statistics are fresh. */
  if (((g_app_camera_status.flags & APP_CAMERA_FLAG_PREVIEW) != 0U) &&
      ((g_app_camera_status.flags & APP_CAMERA_FLAG_STREAMING) != 0U) &&
      (g_app_camera_no_frame_ms == 0U) &&
      (g_app_camera_status.output_bpp == 2U))
  {
    static uint32_t s_last_sample_seq;
    uint32_t seq = g_app_camera_frame_sample_seq;

    if (seq != s_last_sample_seq)
    {
      s_last_sample_seq = seq;
      AppCamera_RunAutoExposure(g_app_camera_frame_luma_avg);
      AppCamera_RunAutoWhiteBalance(g_app_camera_frame_r_avg8,
                                    g_app_camera_frame_g_avg8,
                                    g_app_camera_frame_b_avg8);
    }
  }

  AppCamera_MirrorGlobals();
}

void AppCamera_GetStatus(AppCameraStatus_t *status)
{
  if (status != 0)
  {
    *status = g_app_camera_status;
  }
}

void HAL_DCMIPP_PIPE_FrameEventCallback(DCMIPP_HandleTypeDef *hdcmipp_cb, uint32_t Pipe)
{
  if ((hdcmipp_cb == &hdcmipp) && (Pipe == APP_CAMERA_DCMIPP_PIPE))
  {
    uint32_t active_addr = HAL_DCMIPP_PIPE_GetMemoryAddress(&hdcmipp,
                                                            APP_CAMERA_DCMIPP_PIPE,
                                                            DCMIPP_MEMORY_ADDRESS_0);
    uint32_t completed_addr;

    if (active_addr == APP_CAMERA_FRAME0_ADDR)
    {
      completed_addr = APP_CAMERA_FRAME1_ADDR;
    }
    else if (active_addr == APP_CAMERA_FRAME1_ADDR)
    {
      completed_addr = APP_CAMERA_FRAME0_ADDR;
    }
    else
    {
      completed_addr = ((g_app_camera_status.frame_count & 1U) == 0U) ?
                       APP_CAMERA_FRAME0_ADDR :
                       APP_CAMERA_FRAME1_ADDR;
    }

    g_app_camera_status.completed_frame_addr = completed_addr;
    g_app_camera_completed_addr = completed_addr;
    AppCamera_SampleFrame(completed_addr);
    if ((g_app_camera_status.flags & APP_CAMERA_FLAG_PREVIEW) != 0U)
    {
      AppCameraDisplay_RequestSwap(completed_addr);
      g_app_camera_status.display_addr = g_app_camera_display_addr;
      g_app_camera_status.pending_display_addr = g_app_camera_pending_display_addr;
      g_app_camera_status.ltdc_swap_count = g_app_camera_ltdc_swap_count;
      g_app_camera_status.ltdc_error_count = g_app_camera_ltdc_error_count;
      g_app_camera_status.ltdc_ier2 = g_app_camera_ltdc_ier2;
      g_app_camera_status.ltdc_isr2 = g_app_camera_ltdc_isr2;
      g_app_camera_status.ltdc_layer2_cr = g_app_camera_ltdc_layer2_cr;
      g_app_camera_status.ltdc_auto_disable_count = g_app_camera_ltdc_auto_disable_count;
    }
    g_app_camera_status.frame_count++;
    g_app_camera_status.flags |= APP_CAMERA_FLAG_FRAME_SEEN;
    g_app_camera_frame_count = g_app_camera_status.frame_count;
    g_app_camera_state = g_app_camera_status.flags;
  }
}

void HAL_DCMIPP_PIPE_VsyncEventCallback(DCMIPP_HandleTypeDef *hdcmipp_cb, uint32_t Pipe)
{
  if ((hdcmipp_cb == &hdcmipp) && (Pipe == APP_CAMERA_DCMIPP_PIPE))
  {
    g_app_camera_status.vsync_count++;
  }
}

void HAL_DCMIPP_PIPE_LimitEventCallback(DCMIPP_HandleTypeDef *hdcmipp_cb, uint32_t Pipe)
{
  if ((hdcmipp_cb == &hdcmipp) && (Pipe == APP_CAMERA_DCMIPP_PIPE))
  {
    g_app_camera_status.limit_count++;
  }
}

void HAL_DCMIPP_PIPE_ErrorCallback(DCMIPP_HandleTypeDef *hdcmipp_cb, uint32_t Pipe)
{
  if ((hdcmipp_cb == &hdcmipp) && (Pipe == APP_CAMERA_DCMIPP_PIPE))
  {
    g_app_camera_status.error_count++;
    g_app_camera_status.hal_error = HAL_DCMIPP_GetError(&hdcmipp);
    g_app_camera_status.last_error = APP_CAMERA_ERROR_DCMIPP_START;
  }
}

void HAL_DCMIPP_ErrorCallback(DCMIPP_HandleTypeDef *hdcmipp_cb)
{
  if (hdcmipp_cb == &hdcmipp)
  {
    g_app_camera_status.error_count++;
    g_app_camera_status.hal_error = HAL_DCMIPP_GetError(&hdcmipp);
    g_app_camera_status.last_error = APP_CAMERA_ERROR_DCMIPP_CONFIG;
  }
}

void HAL_DCMIPP_CSI_StartOfFrameEventCallback(DCMIPP_HandleTypeDef *hdcmipp_cb, uint32_t VirtualChannel)
{
  if ((hdcmipp_cb == &hdcmipp) && (VirtualChannel == APP_CAMERA_DCMIPP_VC))
  {
    g_app_camera_status.csi_sof_count++;
  }
}

void HAL_DCMIPP_CSI_EndOfFrameEventCallback(DCMIPP_HandleTypeDef *hdcmipp_cb, uint32_t VirtualChannel)
{
  if ((hdcmipp_cb == &hdcmipp) && (VirtualChannel == APP_CAMERA_DCMIPP_VC))
  {
    g_app_camera_status.csi_eof_count++;
  }
}

void HAL_DCMIPP_CSI_LineErrorCallback(DCMIPP_HandleTypeDef *hdcmipp_cb, uint32_t DataLane)
{
  (void)DataLane;

  if (hdcmipp_cb == &hdcmipp)
  {
    g_app_camera_status.line_error_count++;
    if ((DataLane & DCMIPP_CSI_DATA_LANE0) != 0U)
    {
      g_app_camera_status.lane0_error_count++;
    }
    if ((DataLane & DCMIPP_CSI_DATA_LANE1) != 0U)
    {
      g_app_camera_status.lane1_error_count++;
    }
    g_app_camera_status.error_count++;
    g_app_camera_status.hal_error = HAL_DCMIPP_GetError(&hdcmipp);
    g_app_camera_status.last_error = APP_CAMERA_ERROR_DCMIPP_CONFIG;
  }
}

void HAL_DCMIPP_CSI_ShortPacketDetectionEventCallback(DCMIPP_HandleTypeDef *hdcmipp_cb)
{
  if (hdcmipp_cb == &hdcmipp)
  {
    g_app_camera_status.short_packet_count++;
  }
}

void HAL_DCMIPP_CSI_ClockChangerFifoFullEventCallback(DCMIPP_HandleTypeDef *hdcmipp_cb)
{
  if (hdcmipp_cb == &hdcmipp)
  {
    g_app_camera_status.error_count++;
    g_app_camera_status.hal_error = HAL_DCMIPP_GetError(&hdcmipp);
    g_app_camera_status.last_error = APP_CAMERA_ERROR_DCMIPP_CONFIG;
  }
}
