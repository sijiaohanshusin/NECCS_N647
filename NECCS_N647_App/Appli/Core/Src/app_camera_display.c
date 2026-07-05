#include "app_camera_display.h"

#include "app_camera.h"
#include "main.h"

#define APP_CAMERA_DISPLAY_FLAG_READY (1UL << 0)
#define APP_CAMERA_DISPLAY_FLAG_VISIBLE (1UL << 1)
#define APP_CAMERA_DISPLAY_LTDC_ERROR_MASK \
  (LTDC_ISR2_FUWIF | LTDC_ISR2_TERRIF | LTDC_ISR2_FUIF | LTDC_ISR2_CRCIF)
#define APP_CAMERA_DISPLAY_DCACHE_LINE_BYTES 32U

extern LTDC_HandleTypeDef hltdc;

volatile uint32_t g_app_camera_display_flags = 0U;
volatile uint32_t g_app_camera_display_init_status = APP_CAMERA_DISPLAY_ERROR_LTDC;
volatile uint32_t g_app_camera_display_addr = APP_CAMERA_DISPLAY_TARGET_ADDR;
volatile uint32_t g_app_camera_pending_display_addr = APP_CAMERA_DISPLAY_TARGET_ADDR;
volatile uint32_t g_app_camera_ltdc_swap_count = 0U;
volatile uint32_t g_app_camera_ltdc_error_count = 0U;
volatile uint32_t g_app_camera_ltdc_ier2 = 0U;
volatile uint32_t g_app_camera_ltdc_isr2 = 0U;
volatile uint32_t g_app_camera_ltdc_layer2_cr = 0U;
volatile uint32_t g_app_camera_ltdc_auto_disable_count = 0U;

static int32_t AppCameraDisplay_MaxI32(int32_t a, int32_t b)
{
  return (a > b) ? a : b;
}

static int32_t AppCameraDisplay_MinI32(int32_t a, int32_t b)
{
  return (a < b) ? a : b;
}

static void AppCameraDisplay_CleanDCache(uint32_t address, uint32_t size)
{
  uint32_t aligned_addr;
  uint32_t end_addr;

  if ((size == 0U) || ((SCB->CCR & SCB_CCR_DC_Msk) == 0U))
  {
    return;
  }

  aligned_addr = address & ~(APP_CAMERA_DISPLAY_DCACHE_LINE_BYTES - 1U);
  end_addr = (address + size + APP_CAMERA_DISPLAY_DCACHE_LINE_BYTES - 1U) &
             ~(APP_CAMERA_DISPLAY_DCACHE_LINE_BYTES - 1U);

  SCB_CleanDCache_by_Addr((void *)aligned_addr, (int32_t)(end_addr - aligned_addr));
  __DSB();
  __ISB();
}

static void AppCameraDisplay_SnapshotLtdc(void)
{
  g_app_camera_ltdc_ier2 = LTDC->IER2;
  g_app_camera_ltdc_isr2 = LTDC->ISR2;
  g_app_camera_ltdc_layer2_cr = LTDC_Layer2->CR;
}

static void AppCameraDisplay_ClearLtdcErrors(void)
{
  LTDC->ICR2 = APP_CAMERA_DISPLAY_LTDC_ERROR_MASK;
  AppCameraDisplay_SnapshotLtdc();
}

static void AppCameraDisplay_ReloadLayer(volatile LTDC_Layer_TypeDef *layer, uint32_t reload)
{
  layer->RCR = reload | LTDC_LxRCR_GRMSK;
  LTDC->SRCR = reload;
}

static void AppCameraDisplay_FillRgb565Layer(LTDC_LayerCfgTypeDef *cfg,
                                             uint32_t x0,
                                             uint32_t y0,
                                             uint32_t width,
                                             uint32_t height,
                                             uint32_t address)
{
  cfg->WindowX0 = x0;
  cfg->WindowX1 = x0 + width;
  cfg->WindowY0 = y0;
  cfg->WindowY1 = y0 + height;
  cfg->PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  cfg->Alpha = 255U;
  cfg->Alpha0 = 0U;
  cfg->BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  cfg->BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  cfg->FBStartAdress = address;
  cfg->ImageWidth = width;
  cfg->ImageHeight = height;
  cfg->Backcolor.Blue = 0U;
  cfg->Backcolor.Green = 0U;
  cfg->Backcolor.Red = 0U;
}

static int32_t AppCameraDisplay_ConfigCameraLayer(uint32_t initial_camera_addr)
{
  LTDC_LayerCfgTypeDef layer = {0};

  AppCameraDisplay_FillRgb565Layer(&layer,
                                   APP_CAMERA_DISPLAY_X0,
                                   APP_CAMERA_DISPLAY_Y0,
                                   APP_CAMERA_DISPLAY_WIDTH,
                                   APP_CAMERA_DISPLAY_HEIGHT,
                                   initial_camera_addr);

  if (HAL_LTDC_ConfigLayer(&hltdc, &layer, LTDC_LAYER_1) != HAL_OK)
  {
    return APP_CAMERA_DISPLAY_ERROR_LTDC;
  }

  return APP_CAMERA_DISPLAY_OK;
}

static int32_t AppCameraDisplay_ConfigUiLayer(void)
{
  LTDC_LayerCfgTypeDef layer = {0};

  AppCameraDisplay_FillRgb565Layer(&layer,
                                   0U,
                                   0U,
                                   APP_CAMERA_DISPLAY_SCREEN_WIDTH,
                                   APP_CAMERA_DISPLAY_SCREEN_HEIGHT,
                                   APP_CAMERA_DISPLAY_UI_FB_ADDR);
  layer.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  layer.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;

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

static int32_t AppCameraDisplay_DisableCameraLayer(void)
{
  LTDC_Layer1->CR &= ~LTDC_LxCR_LEN;
  AppCameraDisplay_ReloadLayer(LTDC_Layer1, LTDC_LxRCR_IMR);
  AppCameraDisplay_SnapshotLtdc();

  return APP_CAMERA_DISPLAY_OK;
}

int32_t AppCameraDisplay_InitLayers(uint32_t initial_camera_addr)
{
  int32_t status;
  uint32_t keep_visible = g_app_camera_display_flags & APP_CAMERA_DISPLAY_FLAG_VISIBLE;

  if (initial_camera_addr == 0U)
  {
    g_app_camera_display_init_status = APP_CAMERA_DISPLAY_ERROR_INVALID_ARG;
    g_app_camera_ltdc_error_count++;
    return APP_CAMERA_DISPLAY_ERROR_INVALID_ARG;
  }

  status = AppCameraDisplay_ConfigCameraLayer(initial_camera_addr);
  if (status == APP_CAMERA_DISPLAY_OK)
  {
    status = AppCameraDisplay_ConfigUiLayer();
  }
  if (status == APP_CAMERA_DISPLAY_OK)
  {
    g_app_camera_display_flags |= APP_CAMERA_DISPLAY_FLAG_READY;
    g_app_camera_display_addr = initial_camera_addr;
    g_app_camera_pending_display_addr = initial_camera_addr;
    g_app_camera_display_init_status = APP_CAMERA_DISPLAY_OK;
    AppCameraDisplay_SetVisible((keep_visible != 0U) ? 1U : 0U);
    AppCameraDisplay_ClearLtdcErrors();
  }
  else
  {
    g_app_camera_display_flags &= ~APP_CAMERA_DISPLAY_FLAG_READY;
    (void)AppCameraDisplay_DisableCameraLayer();
    g_app_camera_display_init_status = status;
    g_app_camera_ltdc_error_count++;
  }

  return status;
}

void AppCameraDisplay_SetVisible(uint8_t visible)
{
  if ((g_app_camera_display_flags & APP_CAMERA_DISPLAY_FLAG_READY) == 0U)
  {
    return;
  }

  if (visible != 0U)
  {
    AppCameraDisplay_ClearLtdcErrors();
    g_app_camera_display_flags |= APP_CAMERA_DISPLAY_FLAG_VISIBLE;
    LTDC_Layer1->CR |= LTDC_LxCR_LEN;
    AppCameraDisplay_RefreshColorKeyHole(0,
                                         0,
                                         (int32_t)APP_CAMERA_DISPLAY_SCREEN_WIDTH,
                                         (int32_t)APP_CAMERA_DISPLAY_SCREEN_HEIGHT);
  }
  else
  {
    g_app_camera_display_flags &= ~APP_CAMERA_DISPLAY_FLAG_VISIBLE;
    LTDC_Layer1->CR &= ~LTDC_LxCR_LEN;
  }

  LTDC_Layer2->CR |= (LTDC_LxCR_LEN | LTDC_LxCR_CKEN);
  AppCameraDisplay_ReloadLayer(LTDC_Layer1, LTDC_LxRCR_IMR);
  AppCameraDisplay_ReloadLayer(LTDC_Layer2, LTDC_LxRCR_IMR);
  AppCameraDisplay_SnapshotLtdc();
}

void AppCameraDisplay_RefreshColorKeyHole(int32_t x,
                                          int32_t y,
                                          int32_t width,
                                          int32_t height)
{
  const int32_t hole_x0 = (int32_t)APP_CAMERA_DISPLAY_X0;
  const int32_t hole_y0 =
    AppCameraDisplay_MaxI32((int32_t)APP_CAMERA_DISPLAY_Y0,
                            (int32_t)APP_CAMERA_DISPLAY_TOP_OVERLAY_HEIGHT);
  const int32_t hole_x1 = (int32_t)(APP_CAMERA_DISPLAY_X0 + APP_CAMERA_DISPLAY_WIDTH);
  const int32_t hole_y1 = (int32_t)(APP_CAMERA_DISPLAY_Y0 + APP_CAMERA_DISPLAY_HEIGHT);
  int32_t rect_x0;
  int32_t rect_y0;
  int32_t rect_x1;
  int32_t rect_y1;
  int32_t fill_width;
  int32_t fill_height;
  uint16_t *framebuffer;
  uint16_t *row;
  uint32_t clean_start;
  uint32_t clean_bytes;

  if (((g_app_camera_display_flags & APP_CAMERA_DISPLAY_FLAG_VISIBLE) == 0U) ||
      (width <= 0) ||
      (height <= 0))
  {
    return;
  }

  rect_x0 = AppCameraDisplay_MaxI32(x, 0);
  rect_y0 = AppCameraDisplay_MaxI32(y, 0);
  rect_x1 = AppCameraDisplay_MinI32(x + width, (int32_t)APP_CAMERA_DISPLAY_SCREEN_WIDTH);
  rect_y1 = AppCameraDisplay_MinI32(y + height, (int32_t)APP_CAMERA_DISPLAY_SCREEN_HEIGHT);

  rect_x0 = AppCameraDisplay_MaxI32(rect_x0, hole_x0);
  rect_y0 = AppCameraDisplay_MaxI32(rect_y0, hole_y0);
  rect_x1 = AppCameraDisplay_MinI32(rect_x1, hole_x1);
  rect_y1 = AppCameraDisplay_MinI32(rect_y1, hole_y1);

  fill_width = rect_x1 - rect_x0;
  fill_height = rect_y1 - rect_y0;
  if ((fill_width <= 0) || (fill_height <= 0))
  {
    return;
  }

  framebuffer = (uint16_t *)APP_CAMERA_DISPLAY_UI_FB_ADDR;
  row = framebuffer + ((uint32_t)rect_y0 * APP_CAMERA_DISPLAY_SCREEN_WIDTH) + (uint32_t)rect_x0;
  for (int32_t yy = 0; yy < fill_height; yy++)
  {
    uint16_t *pixel = row + ((uint32_t)yy * APP_CAMERA_DISPLAY_SCREEN_WIDTH);
    for (int32_t xx = 0; xx < fill_width; xx++)
    {
      pixel[xx] = APP_CAMERA_DISPLAY_COLOR_KEY_RGB565;
    }
  }

  clean_start = (uint32_t)row;
  clean_bytes = ((((uint32_t)fill_height - 1U) * APP_CAMERA_DISPLAY_SCREEN_WIDTH) +
                 (uint32_t)fill_width) *
                APP_CAMERA_DISPLAY_BYTES_PER_PIXEL;
  AppCameraDisplay_CleanDCache(clean_start, clean_bytes);
}

void AppCameraDisplay_RequestSwap(uint32_t frame_addr)
{
  if (((g_app_camera_display_flags & APP_CAMERA_DISPLAY_FLAG_READY) == 0U) || (frame_addr == 0U))
  {
    return;
  }

  AppCameraDisplay_SnapshotLtdc();
  if ((g_app_camera_ltdc_isr2 & APP_CAMERA_DISPLAY_LTDC_ERROR_MASK) != 0U)
  {
    g_app_camera_ltdc_error_count++;
    g_app_camera_ltdc_auto_disable_count++;
    AppCameraDisplay_SetVisible(0U);
    AppCameraDisplay_ClearLtdcErrors();
    return;
  }

  g_app_camera_pending_display_addr = frame_addr;
  g_app_camera_display_addr = frame_addr;
  if ((g_app_camera_display_flags & APP_CAMERA_DISPLAY_FLAG_VISIBLE) != 0U)
  {
    LTDC_Layer1->CFBAR = frame_addr;
    AppCameraDisplay_ReloadLayer(LTDC_Layer1, LTDC_LxRCR_VBR);
  }
  g_app_camera_ltdc_swap_count++;
  AppCameraDisplay_SnapshotLtdc();
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
    status->ltdc_ier2 = g_app_camera_ltdc_ier2;
    status->ltdc_isr2 = g_app_camera_ltdc_isr2;
    status->ltdc_layer2_cr = g_app_camera_ltdc_layer2_cr;
    status->auto_disable_count = g_app_camera_ltdc_auto_disable_count;
    status->init_status = (int32_t)g_app_camera_display_init_status;
  }
}
