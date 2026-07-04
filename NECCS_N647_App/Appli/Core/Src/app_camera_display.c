#include "app_camera_display.h"

#include "app_camera.h"
#include "main.h"

#define APP_CAMERA_DISPLAY_FLAG_READY (1UL << 0)

extern LTDC_HandleTypeDef hltdc;

volatile uint32_t g_app_camera_display_flags = 0U;
volatile uint32_t g_app_camera_display_init_status = APP_CAMERA_DISPLAY_ERROR_LTDC;
volatile uint32_t g_app_camera_display_addr = APP_CAMERA_FRAME0_ADDR;
volatile uint32_t g_app_camera_pending_display_addr = APP_CAMERA_FRAME0_ADDR;
volatile uint32_t g_app_camera_ltdc_swap_count = 0U;
volatile uint32_t g_app_camera_ltdc_error_count = 0U;

static int32_t AppCameraDisplay_ConfigCameraLayer(uint32_t initial_camera_addr)
{
  LTDC_LayerCfgTypeDef layer = {0};

  layer.WindowX0 = APP_CAMERA_DISPLAY_X0;
  layer.WindowX1 = APP_CAMERA_DISPLAY_X0 + APP_CAMERA_DISPLAY_WIDTH;
  layer.WindowY0 = APP_CAMERA_DISPLAY_Y0;
  layer.WindowY1 = APP_CAMERA_DISPLAY_Y0 + APP_CAMERA_DISPLAY_HEIGHT;
  layer.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  layer.Alpha = 255U;
  layer.Alpha0 = 0U;
  layer.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  layer.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  layer.FBStartAdress = initial_camera_addr;
  layer.ImageWidth = APP_CAMERA_DISPLAY_WIDTH;
  layer.ImageHeight = APP_CAMERA_DISPLAY_HEIGHT;
  layer.Backcolor.Blue = 0U;
  layer.Backcolor.Green = 0U;
  layer.Backcolor.Red = 0U;

  if (HAL_LTDC_ConfigLayer(&hltdc, &layer, LTDC_LAYER_1) != HAL_OK)
  {
    return APP_CAMERA_DISPLAY_ERROR_LTDC;
  }

  return APP_CAMERA_DISPLAY_OK;
}

static int32_t AppCameraDisplay_ConfigOverlayLayer(void)
{
  LTDC_LayerCfgTypeDef layer = {0};

  layer.WindowX0 = 0U;
  layer.WindowX1 = 1024U;
  layer.WindowY0 = 0U;
  layer.WindowY1 = 600U;
  layer.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  layer.Alpha = 255U;
  layer.Alpha0 = 0U;
  layer.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  layer.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  layer.FBStartAdress = APP_CAMERA_DISPLAY_UI_FB_ADDR;
  layer.ImageWidth = 1024U;
  layer.ImageHeight = 600U;
  layer.Backcolor.Blue = 0U;
  layer.Backcolor.Green = 0U;
  layer.Backcolor.Red = 0U;

  if (HAL_LTDC_ConfigLayer(&hltdc, &layer, LTDC_LAYER_2) != HAL_OK)
  {
    return APP_CAMERA_DISPLAY_ERROR_LTDC;
  }
  if (HAL_LTDC_ConfigColorKeying(&hltdc, APP_CAMERA_DISPLAY_COLOR_KEY_RGB888, LTDC_LAYER_2) != HAL_OK)
  {
    return APP_CAMERA_DISPLAY_ERROR_LTDC;
  }
  if (HAL_LTDC_EnableColorKeying(&hltdc, LTDC_LAYER_2) != HAL_OK)
  {
    return APP_CAMERA_DISPLAY_ERROR_LTDC;
  }

  return APP_CAMERA_DISPLAY_OK;
}

int32_t AppCameraDisplay_InitLayers(uint32_t initial_camera_addr)
{
  int32_t status;

  if (initial_camera_addr == 0U)
  {
    g_app_camera_display_init_status = APP_CAMERA_DISPLAY_ERROR_INVALID_ARG;
    g_app_camera_ltdc_error_count++;
    return APP_CAMERA_DISPLAY_ERROR_INVALID_ARG;
  }

  status = AppCameraDisplay_ConfigCameraLayer(initial_camera_addr);
  if (status == APP_CAMERA_DISPLAY_OK)
  {
    status = AppCameraDisplay_ConfigOverlayLayer();
  }

  if (status == APP_CAMERA_DISPLAY_OK)
  {
    g_app_camera_display_flags |= APP_CAMERA_DISPLAY_FLAG_READY;
    g_app_camera_display_addr = initial_camera_addr;
    g_app_camera_pending_display_addr = initial_camera_addr;
    g_app_camera_display_init_status = APP_CAMERA_DISPLAY_OK;
  }
  else
  {
    g_app_camera_display_flags &= ~APP_CAMERA_DISPLAY_FLAG_READY;
    g_app_camera_display_init_status = status;
    g_app_camera_ltdc_error_count++;
  }

  return status;
}

void AppCameraDisplay_RequestSwap(uint32_t frame_addr)
{
  if (((g_app_camera_display_flags & APP_CAMERA_DISPLAY_FLAG_READY) == 0U) || (frame_addr == 0U))
  {
    return;
  }

  g_app_camera_pending_display_addr = frame_addr;
  LTDC_Layer1->CFBAR = frame_addr;
  LTDC->SRCR = LTDC_SRCR_VBR;
  g_app_camera_display_addr = frame_addr;
  g_app_camera_ltdc_swap_count++;
}

void AppCameraDisplay_GetStatus(AppCameraDisplayStatus_t *status)
{
  if (status != 0)
  {
    status->flags = g_app_camera_display_flags;
    status->display_addr = g_app_camera_display_addr;
    status->pending_display_addr = g_app_camera_pending_display_addr;
    status->swap_count = g_app_camera_ltdc_swap_count;
    status->error_count = g_app_camera_ltdc_error_count;
    status->init_status = (int32_t)g_app_camera_display_init_status;
  }
}
