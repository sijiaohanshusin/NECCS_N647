#include "app_camera.h"

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
  .pipe_index = APP_CAMERA_DCMIPP_PIPE,
  .phy_bitrate = APP_CAMERA_PRIMARY_PHY,
};

static uint32_t g_app_camera_last_frame_poll;
static uint32_t g_app_camera_no_frame_ms;
static uint32_t g_app_camera_phy_retry_index;
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

  SCB_InvalidateDCache_by_Addr((void *)aligned_addr, (int32_t)(end_addr - aligned_addr));
}

static void AppCamera_PrepareBuffers(void)
{
  (void)memset((void *)APP_CAMERA_FRAME0_ADDR, 0, APP_CAMERA_FRAME_BUFFER_BYTES);
  (void)memset((void *)APP_CAMERA_FRAME1_ADDR, 0, APP_CAMERA_FRAME_BUFFER_BYTES);
  AppCamera_CacheCleanInvalidate(APP_CAMERA_FRAME0_ADDR, APP_CAMERA_FRAME_BUFFER_BYTES);
  AppCamera_CacheCleanInvalidate(APP_CAMERA_FRAME1_ADDR, APP_CAMERA_FRAME_BUFFER_BYTES);
}

static int32_t AppCamera_ConfigureDcmipp(uint32_t phy_bitrate)
{
  DCMIPP_CSI_ConfTypeDef csi_config = {0};
  DCMIPP_CSI_VCFilteringConfTypeDef vc_filter_config = {0};
  DCMIPP_CSI_PIPE_ConfTypeDef csi_pipe_config = {0};
  DCMIPP_PipeConfTypeDef pipe_config = {0};
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
  pipe_config.PixelPipePitch = APP_CAMERA_CAPTURE_LINE_BYTES;
  pipe_config.PixelPackerFormat = DCMIPP_PIXEL_PACKER_FORMAT_MONO_Y8_G8_1;
  hal_status = HAL_DCMIPP_PIPE_SetConfig(&hdcmipp, APP_CAMERA_DCMIPP_PIPE, &pipe_config);
  if (hal_status != HAL_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_DCMIPP_CONFIG);
    return APP_CAMERA_ERROR_DCMIPP_CONFIG;
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

static int32_t AppCamera_StartDcmippCapture(void)
{
  HAL_StatusTypeDef hal_status;

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

  status = AppCamera_ConfigureDcmipp(phy_bitrate);
  if (status != APP_CAMERA_OK)
  {
    AppCamera_SetError(APP_CAMERA_ERROR_FALLBACK_RESTART);
    return status;
  }

  status = AppCamera_StartDcmippCapture();
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
  g_app_camera_status.frame_bytes = APP_CAMERA_CAPTURE_FRAME_BYTES;
  g_app_camera_status.frame_buffer_bytes = APP_CAMERA_FRAME_BUFFER_BYTES;
  g_app_camera_status.pipe_index = APP_CAMERA_DCMIPP_PIPE;
  g_app_camera_status.phy_bitrate = APP_CAMERA_PRIMARY_PHY;
  g_app_camera_status.fallback_count = 0U;
  g_app_camera_status.poll_count = 0U;
  g_app_camera_last_frame_poll = 0U;
  g_app_camera_no_frame_ms = 0U;
  g_app_camera_phy_retry_index = 0U;

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

int32_t AppCamera_StartSmoke(void)
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

  if ((g_app_camera_status.flags & APP_CAMERA_FLAG_STREAMING) != 0U)
  {
    g_app_camera_start_status = APP_CAMERA_OK;
    return APP_CAMERA_OK;
  }

  AppCamera_PrepareBuffers();

  status = AppCamera_ConfigureDcmipp(APP_CAMERA_PRIMARY_PHY);
  if (status != APP_CAMERA_OK)
  {
    g_app_camera_start_status = (uint32_t)status;
    return status;
  }

  status = AppCamera_StartDcmippCapture();
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

void AppCamera_Poll(uint32_t elapsed_ms)
{
  uint32_t data_counter = 0U;

  g_app_camera_status.poll_count++;

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
    AppCamera_CacheInvalidate(APP_CAMERA_FRAME0_ADDR, APP_CAMERA_FRAME_BUFFER_BYTES);
    AppCamera_CacheInvalidate(APP_CAMERA_FRAME1_ADDR, APP_CAMERA_FRAME_BUFFER_BYTES);
    g_app_camera_last_frame_poll = g_app_camera_status.frame_count;
    g_app_camera_no_frame_ms = 0U;
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
    g_app_camera_status.frame_count++;
    g_app_camera_status.flags |= APP_CAMERA_FLAG_FRAME_SEEN;
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
