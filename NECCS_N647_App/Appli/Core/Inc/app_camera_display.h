#ifndef APP_CAMERA_DISPLAY_H
#define APP_CAMERA_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APP_CAMERA_DISPLAY_OK                 0
#define APP_CAMERA_DISPLAY_ERROR_INVALID_ARG -1
#define APP_CAMERA_DISPLAY_ERROR_LTDC        -2

#define APP_CAMERA_DISPLAY_X0                 192U
#define APP_CAMERA_DISPLAY_Y0                 60U
#define APP_CAMERA_DISPLAY_WIDTH              640U
#define APP_CAMERA_DISPLAY_HEIGHT             480U
#define APP_CAMERA_DISPLAY_UI_FB_ADDR         0x90000000UL
#define APP_CAMERA_DISPLAY_COLOR_KEY_RGB888   0x00FF00FFUL
#define APP_CAMERA_DISPLAY_COLOR_KEY_RGB565   0xF81FU

typedef struct
{
  uint32_t flags;
  uint32_t display_addr;
  uint32_t pending_display_addr;
  uint32_t swap_count;
  uint32_t error_count;
  int32_t init_status;
} AppCameraDisplayStatus_t;

extern volatile uint32_t g_app_camera_display_flags;
extern volatile uint32_t g_app_camera_display_init_status;
extern volatile uint32_t g_app_camera_display_addr;
extern volatile uint32_t g_app_camera_pending_display_addr;
extern volatile uint32_t g_app_camera_ltdc_swap_count;
extern volatile uint32_t g_app_camera_ltdc_error_count;

int32_t AppCameraDisplay_InitLayers(uint32_t initial_camera_addr);
void AppCameraDisplay_RequestSwap(uint32_t frame_addr);
void AppCameraDisplay_GetStatus(AppCameraDisplayStatus_t *status);

#ifdef __cplusplus
}
#endif

#endif /* APP_CAMERA_DISPLAY_H */
