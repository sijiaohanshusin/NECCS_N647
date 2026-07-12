#ifndef APP_CAMERA_H
#define APP_CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "app_camera_display.h"

#define APP_CAMERA_WIDTH              640U
#define APP_CAMERA_HEIGHT             480U
#define APP_CAMERA_FPS                15U
#define APP_CAMERA_INPUT_CLOCK_HZ     24000000UL
#define APP_CAMERA_FRAME0_ADDR        APP_CAMERA_DISPLAY_CAMERA_FB0_ADDR
#define APP_CAMERA_FRAME1_ADDR        APP_CAMERA_DISPLAY_CAMERA_FB1_ADDR
#define APP_CAMERA_RAW10_FRAME_BYTES  (((APP_CAMERA_WIDTH * APP_CAMERA_HEIGHT * 10U) + 7U) / 8U)
#define APP_CAMERA_PREVIEW_LINE_BYTES APP_CAMERA_DISPLAY_CAMERA_LINE_BYTES
#define APP_CAMERA_PREVIEW_FRAME_BYTES APP_CAMERA_DISPLAY_CAMERA_FRAME_BYTES
#define APP_CAMERA_CAPTURE_LINE_BYTES APP_CAMERA_WIDTH
#define APP_CAMERA_CAPTURE_FRAME_BYTES (APP_CAMERA_CAPTURE_LINE_BYTES * APP_CAMERA_HEIGHT)
#define APP_CAMERA_FRAME_BUFFER_BYTES APP_CAMERA_DISPLAY_CAMERA_BUFFER_BYTES

#define APP_CAMERA_OK                         0
#define APP_CAMERA_ERROR_INVALID_ARG         -1
#define APP_CAMERA_ERROR_SENSOR_INIT         -2
#define APP_CAMERA_ERROR_DCMIPP_INIT         -3
#define APP_CAMERA_ERROR_DCMIPP_CONFIG       -4
#define APP_CAMERA_ERROR_DCMIPP_START        -5
#define APP_CAMERA_ERROR_SENSOR_STREAM       -6
#define APP_CAMERA_ERROR_NOT_INITIALIZED     -7
#define APP_CAMERA_ERROR_FALLBACK_RESTART    -8
#define APP_CAMERA_ERROR_DISPLAY_INIT        -9
#define APP_CAMERA_ERROR_TEST_PATTERN       -10

#define APP_CAMERA_FLAG_POWERED              (1UL << 0)
#define APP_CAMERA_FLAG_I2C_OK               (1UL << 1)
#define APP_CAMERA_FLAG_CHIP_ID_OK           (1UL << 2)
#define APP_CAMERA_FLAG_SENSOR_CONFIGURED    (1UL << 3)
#define APP_CAMERA_FLAG_DCMIPP_READY         (1UL << 4)
#define APP_CAMERA_FLAG_STREAMING            (1UL << 5)
#define APP_CAMERA_FLAG_FRAME_SEEN           (1UL << 6)
#define APP_CAMERA_FLAG_FALLBACK_PHY         (1UL << 7)
#define APP_CAMERA_FLAG_PREVIEW              (1UL << 8)
#define APP_CAMERA_FLAG_DISPLAY_READY        (1UL << 9)
#define APP_CAMERA_FLAG_TEST_PATTERN         (1UL << 10)

typedef struct
{
  uint32_t flags;
  uint16_t chip_id;
  uint16_t width;
  uint16_t height;
  uint16_t fps;
  uint32_t frame_count;
  uint32_t csi_sof_count;
  uint32_t csi_eof_count;
  uint32_t vsync_count;
  uint32_t limit_count;
  uint32_t line_error_count;
  uint32_t lane0_error_count;
  uint32_t lane1_error_count;
  uint32_t short_packet_count;
  uint32_t error_count;
  int32_t last_error;
  uint32_t hal_error;
  uint32_t data_counter;
  uint32_t frame0_addr;
  uint32_t frame1_addr;
  uint32_t frame_bytes;
  uint32_t frame_buffer_bytes;
  uint32_t output_format;
  uint32_t output_bpp;
  uint32_t line_pitch;
  uint32_t bayer_type;
  uint32_t test_pattern_enabled;
  uint32_t completed_frame_addr;
  uint32_t display_addr;
  uint32_t pending_display_addr;
  uint32_t ltdc_swap_count;
  uint32_t ltdc_error_count;
  uint32_t ltdc_ier2;
  uint32_t ltdc_isr2;
  uint32_t ltdc_layer2_cr;
  uint32_t ltdc_auto_disable_count;
  uint32_t pipe_index;
  uint32_t phy_bitrate;
  uint32_t fallback_count;
  uint32_t poll_count;
} AppCameraStatus_t;

extern volatile uint32_t g_app_camera_state;
extern volatile uint32_t g_app_camera_init_status;
extern volatile uint32_t g_app_camera_start_status;
extern volatile uint32_t g_app_camera_chip_id;
extern volatile uint32_t g_app_camera_frame_count;
extern volatile uint32_t g_app_camera_csi_sof_count;
extern volatile uint32_t g_app_camera_csi_eof_count;
extern volatile uint32_t g_app_camera_vsync_count;
extern volatile uint32_t g_app_camera_limit_count;
extern volatile uint32_t g_app_camera_line_error_count;
extern volatile uint32_t g_app_camera_lane0_error_count;
extern volatile uint32_t g_app_camera_lane1_error_count;
extern volatile uint32_t g_app_camera_short_packet_count;
extern volatile uint32_t g_app_camera_error_count;
extern volatile uint32_t g_app_camera_last_error;
extern volatile uint32_t g_app_camera_hal_error;
extern volatile uint32_t g_app_camera_data_counter;
extern volatile uint32_t g_app_camera_phy_bitrate;
extern volatile uint32_t g_app_camera_fallback_count;
extern volatile uint32_t g_app_camera_frame0_addr;
extern volatile uint32_t g_app_camera_frame1_addr;
extern volatile uint32_t g_app_camera_frame_bytes;
extern volatile uint32_t g_app_camera_output_format;
extern volatile uint32_t g_app_camera_line_pitch;
extern volatile uint32_t g_app_camera_bayer_type;
extern volatile uint32_t g_app_camera_test_pattern_enabled;
extern volatile uint32_t g_app_camera_completed_addr;
extern volatile uint32_t g_app_camera_display_addr;
extern volatile uint32_t g_app_camera_pending_display_addr;
extern volatile uint32_t g_app_camera_ltdc_swap_count;
extern volatile uint32_t g_app_camera_ltdc_error_count;
extern volatile uint32_t g_app_camera_ltdc_ier2;
extern volatile uint32_t g_app_camera_ltdc_isr2;
extern volatile uint32_t g_app_camera_ltdc_layer2_cr;
extern volatile uint32_t g_app_camera_ltdc_auto_disable_count;
extern volatile uint32_t g_app_camera_debug_keepalive;
extern volatile uint32_t g_app_camera_csi_sr0;
extern volatile uint32_t g_app_camera_csi_sr1;
extern volatile uint32_t g_app_camera_csi_err1;
extern volatile uint32_t g_app_camera_csi_err2;
extern volatile uint32_t g_app_camera_csi_spdfr;
extern volatile uint32_t g_app_camera_csi_vc0cfgr1;
extern volatile uint32_t g_app_camera_csi_vc0cfgr2;
extern volatile uint32_t g_app_camera_csi_lmcfgr;
extern volatile uint32_t g_app_camera_csi_pcr;
extern volatile uint32_t g_app_camera_csi_prcr;
extern volatile uint32_t g_app_camera_csi_pmcr;
extern volatile uint32_t g_app_camera_csi_ier0;
extern volatile uint32_t g_app_camera_csi_ier1;
extern volatile uint32_t g_app_camera_csi_pfcr;
extern volatile uint32_t g_app_camera_dcmipp_cmier;
extern volatile uint32_t g_app_camera_dcmipp_cmsr1;
extern volatile uint32_t g_app_camera_dcmipp_cmsr2;
extern volatile uint32_t g_app_camera_dcmipp_p0fscr;
extern volatile uint32_t g_app_camera_dcmipp_p0fctcr;
extern volatile uint32_t g_app_camera_dcmipp_p0dccntr;
extern volatile uint32_t g_app_camera_dcmipp_p0dclmtr;
extern volatile uint32_t g_app_camera_dcmipp_p0ppcr;
extern volatile uint32_t g_app_camera_frame_sample_addr;
extern volatile uint32_t g_app_camera_frame_sample_min;
extern volatile uint32_t g_app_camera_frame_sample_max;
extern volatile uint32_t g_app_camera_frame_sample_avg;
extern volatile uint32_t g_app_camera_frame_sample_non_dark_count;
extern volatile uint32_t g_app_camera_frame_sample_change_count;
extern volatile uint32_t g_app_camera_frame_sample_center;
extern volatile uint32_t g_app_camera_frame_sample_seq;

int32_t AppCamera_Init(void);
int32_t AppCamera_StartSmoke(void);
int32_t AppCamera_StartPreview(void);
int32_t AppCamera_Stop(void);
int32_t AppCamera_SetTestPattern(uint8_t enable);
void AppCamera_Poll(uint32_t elapsed_ms);
/* Debug screenshot hook: parks the capture pipe when g_app_camera_freeze_request
 * is set AND this frame carries the acoustic overlay; called by the display
 * swap worker after its overlay pass. */
extern volatile uint32_t g_app_camera_freeze_request;
void AppCamera_FreezeIfRequested(uint8_t overlay_drawn);
uint8_t AppCamera_IsFrozen(void);
void AppCamera_GetStatus(AppCameraStatus_t *status);

#ifdef __cplusplus
}
#endif

#endif /* APP_CAMERA_H */
