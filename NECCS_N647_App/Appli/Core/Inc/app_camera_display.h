#ifndef APP_CAMERA_DISPLAY_H
#define APP_CAMERA_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "tx_api.h"

#define APP_CAMERA_DISPLAY_OK                 0
#define APP_CAMERA_DISPLAY_ERROR_INVALID_ARG -1
#define APP_CAMERA_DISPLAY_ERROR_LTDC        -2

#define APP_CAMERA_DISPLAY_X0                 192U
#define APP_CAMERA_DISPLAY_Y0                 60U
#define APP_CAMERA_DISPLAY_WIDTH              640U
#define APP_CAMERA_DISPLAY_HEIGHT             480U
#define APP_CAMERA_DISPLAY_SCREEN_WIDTH       1024U
#define APP_CAMERA_DISPLAY_SCREEN_HEIGHT      600U
#define APP_CAMERA_DISPLAY_TOP_OVERLAY_HEIGHT 64U
#define APP_CAMERA_DISPLAY_BYTES_PER_PIXEL    2U
#define APP_CAMERA_DISPLAY_UI_FB_ADDR         0x90072000UL
#define APP_CAMERA_DISPLAY_CAMERA_FB0_ADDR    0x90400000UL
#define APP_CAMERA_DISPLAY_CAMERA_FB1_ADDR    0x90500000UL
#define APP_CAMERA_DISPLAY_UI_LINE_BYTES \
  (APP_CAMERA_DISPLAY_SCREEN_WIDTH * APP_CAMERA_DISPLAY_BYTES_PER_PIXEL)
#define APP_CAMERA_DISPLAY_CAMERA_LINE_BYTES \
  (APP_CAMERA_DISPLAY_WIDTH * APP_CAMERA_DISPLAY_BYTES_PER_PIXEL)
#define APP_CAMERA_DISPLAY_CAMERA_FRAME_BYTES \
  (APP_CAMERA_DISPLAY_CAMERA_LINE_BYTES * APP_CAMERA_DISPLAY_HEIGHT)
#define APP_CAMERA_DISPLAY_CAMERA_BUFFER_BYTES 0x000A0000UL
#define APP_CAMERA_DISPLAY_TARGET_ADDR        APP_CAMERA_DISPLAY_CAMERA_FB0_ADDR
#define APP_CAMERA_DISPLAY_COLOR_KEY_RGB888   0x00FF00FFUL
#define APP_CAMERA_DISPLAY_COLOR_KEY_RGB565   0xF81FU

/* Acoustic heat field pushed by the UI Model (camera-aspect, 48x36). */
#define APP_CAMERA_DISPLAY_FIELD_W            48U
#define APP_CAMERA_DISPLAY_FIELD_H            36U
#define APP_CAMERA_DISPLAY_FIELD_COUNT \
  (APP_CAMERA_DISPLAY_FIELD_W * APP_CAMERA_DISPLAY_FIELD_H)
#define APP_CAMERA_DISPLAY_MARKER_MAX         3U

/* Source marker in camera-frame pixel coordinates. */
typedef struct
{
  uint16_t x;
  uint16_t y;
  uint8_t strength;
} AppCameraDisplayMarker_t;

typedef struct
{
  uint32_t flags;
  uint32_t display_addr;
  uint32_t pending_display_addr;
  uint32_t swap_count;
  uint32_t error_count;
  uint32_t ltdc_ier2;
  uint32_t ltdc_isr2;
  uint32_t ltdc_layer1_cr;
  uint32_t ltdc_layer1_cfbar;
  uint32_t ltdc_layer2_cr;
  uint32_t ltdc_layer2_cfbar;
  uint32_t ui_fb_addr;
  uint32_t auto_disable_count;
  uint32_t overlay_update_count;
  uint32_t overlay_draw_count;
  uint32_t dma2d_copy_count;
  uint32_t dma2d_fallback_count;
  uint32_t dma2d_error_code;
  int32_t init_status;
} AppCameraDisplayStatus_t;

extern volatile uint32_t g_app_camera_display_flags;
extern volatile uint32_t g_app_camera_display_init_status;
extern volatile uint32_t g_app_camera_display_addr;
extern volatile uint32_t g_app_camera_pending_display_addr;
extern volatile uint32_t g_app_camera_ltdc_swap_count;
extern volatile uint32_t g_app_camera_ltdc_error_count;
extern volatile uint32_t g_app_camera_ltdc_ier2;
extern volatile uint32_t g_app_camera_ltdc_isr2;
extern volatile uint32_t g_app_camera_ltdc_layer1_cr;
extern volatile uint32_t g_app_camera_ltdc_layer1_cfbar;
extern volatile uint32_t g_app_camera_ltdc_layer2_cr;
extern volatile uint32_t g_app_camera_ltdc_layer2_cfbar;
extern volatile uint32_t g_app_camera_ui_fb_addr;
extern volatile uint32_t g_app_camera_ltdc_auto_disable_count;
extern volatile uint32_t g_app_camera_overlay_update_count;
extern volatile uint32_t g_app_camera_overlay_draw_count;
extern volatile uint32_t g_app_camera_overlay_draw_cycles;
extern volatile uint32_t g_app_camera_dma2d_copy_count;
extern volatile uint32_t g_app_camera_dma2d_fallback_count;
extern volatile uint32_t g_app_camera_dma2d_error_code;

int32_t AppCameraDisplay_InitLayers(uint32_t initial_camera_addr);
void AppCameraDisplay_SetVisible(uint8_t visible);
void AppCameraDisplay_SetAcousticField(const uint8_t *field,
                                       uint32_t count,
                                       const AppCameraDisplayMarker_t *markers,
                                       uint8_t marker_count,
                                       uint8_t quality_pct,
                                       uint8_t enabled);
void AppCameraDisplay_SetHeatPalette(uint8_t palette);
uint8_t AppCameraDisplay_GetHeatPalette(void);
/* Copy the currently displayed camera frame (with heat overlay) into dst
 * (APP_CAMERA_DISPLAY_WIDTH x HEIGHT RGB565). Returns 1 when the layer is
 * visible and a frame was copied; used by the screenshot compositor. */
uint8_t AppCameraDisplay_CopyDisplayedFrame(uint16_t *dst);
void AppCameraDisplay_RequestSwap(uint32_t frame_addr);
/* Swap worker thread: does the compose/overlay/LTDC work queued by
 * RequestSwap (which is ISR-safe and only records the newest frame). */
UINT AppCameraDisplay_WorkerInit(void);
void AppCameraDisplay_WorkerThreadEntry(ULONG thread_input);
void AppCameraDisplay_RefreshColorKeyHole(int32_t x,
                                          int32_t y,
                                          int32_t width,
                                          int32_t height);
void AppCameraDisplay_GetStatus(AppCameraDisplayStatus_t *status);

#ifdef __cplusplus
}
#endif

#endif /* APP_CAMERA_DISPLAY_H */
