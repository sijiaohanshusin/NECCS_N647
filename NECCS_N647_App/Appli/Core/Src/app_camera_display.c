#include "app_camera_display.h"

#include "app_camera.h"
#include "main.h"
#include <string.h>

#define APP_CAMERA_DISPLAY_FLAG_READY (1UL << 0)
#define APP_CAMERA_DISPLAY_FLAG_VISIBLE (1UL << 1)
#define APP_CAMERA_DISPLAY_LTDC_ERROR_MASK \
  (LTDC_ISR2_FUWIF | LTDC_ISR2_TERRIF | LTDC_ISR2_FUIF | LTDC_ISR2_CRCIF)
#define APP_CAMERA_DISPLAY_DCACHE_LINE_BYTES 32U
#define APP_CAMERA_DISPLAY_HEAT_GRID_SIZE    9U
#define APP_CAMERA_DISPLAY_HEAT_CELL_COUNT \
  (APP_CAMERA_DISPLAY_HEAT_GRID_SIZE * APP_CAMERA_DISPLAY_HEAT_GRID_SIZE)
#define APP_CAMERA_DISPLAY_HEAT_MIN_VALUE    9U
#define APP_CAMERA_DISPLAY_OVERLAY_BRINGUP_ENABLE 1U
#define APP_CAMERA_DISPLAY_BRINGUP_PEAK_VALUE     224U
#define APP_CAMERA_DISPLAY_COMPOSE_ENABLE          1U

extern LTDC_HandleTypeDef hltdc;

typedef struct
{
  uint8_t enabled;
  uint8_t valid;
  uint8_t peak_index;
  uint8_t quality_pct;
  uint8_t heat[APP_CAMERA_DISPLAY_HEAT_CELL_COUNT];
} AppCameraDisplayAcousticOverlay_t;

volatile uint32_t g_app_camera_display_flags = 0U;
volatile uint32_t g_app_camera_display_init_status = APP_CAMERA_DISPLAY_ERROR_LTDC;
volatile uint32_t g_app_camera_display_addr = APP_CAMERA_DISPLAY_TARGET_ADDR;
volatile uint32_t g_app_camera_pending_display_addr = APP_CAMERA_DISPLAY_TARGET_ADDR;
volatile uint32_t g_app_camera_ltdc_swap_count = 0U;
volatile uint32_t g_app_camera_ltdc_error_count = 0U;
volatile uint32_t g_app_camera_ltdc_ier2 = 0U;
volatile uint32_t g_app_camera_ltdc_isr2 = 0U;
volatile uint32_t g_app_camera_ltdc_layer1_cr = 0U;
volatile uint32_t g_app_camera_ltdc_layer1_cfbar = 0U;
volatile uint32_t g_app_camera_ltdc_layer2_cr = 0U;
volatile uint32_t g_app_camera_ltdc_layer2_cfbar = 0U;
volatile uint32_t g_app_camera_ui_fb_addr = APP_CAMERA_DISPLAY_UI_FB_ADDR;
volatile uint32_t g_app_camera_ltdc_auto_disable_count = 0U;
volatile uint32_t g_app_camera_overlay_update_count = 0U;
volatile uint32_t g_app_camera_overlay_draw_count = 0U;
volatile uint32_t g_app_camera_compose_addr = 0U;
volatile uint32_t g_app_camera_compose_count = 0U;

static AppCameraDisplayAcousticOverlay_t s_acoustic_overlay;
static uint16_t s_camera_display_compose_buffer[APP_CAMERA_DISPLAY_WIDTH * APP_CAMERA_DISPLAY_HEIGHT]
  __attribute__((section(".EXTRAM"), aligned(32)));

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

static void AppCameraDisplay_InvalidateDCache(uint32_t address, uint32_t size)
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

  SCB_InvalidateDCache_by_Addr((void *)aligned_addr, (int32_t)(end_addr - aligned_addr));
  __DSB();
  __ISB();
}

static void AppCameraDisplay_CleanInvalidateDCache(uint32_t address, uint32_t size)
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

  SCB_CleanInvalidateDCache_by_Addr((void *)aligned_addr, (int32_t)(end_addr - aligned_addr));
  __DSB();
  __ISB();
}

static uint32_t AppCameraDisplay_GetUiFramebufferAddr(void)
{
  uint32_t address = LTDC_Layer2->CFBAR;

  if (address == 0U)
  {
    address = APP_CAMERA_DISPLAY_UI_FB_ADDR;
  }

  g_app_camera_ui_fb_addr = address;
  return address;
}

static void AppCameraDisplay_FlushCameraRect(uint32_t frame_addr,
                                             int32_t x0,
                                             int32_t y0,
                                             int32_t x1,
                                             int32_t y1,
                                             uint8_t invalidate)
{
  uint32_t address;
  uint32_t bytes;
  uint32_t width;
  uint32_t height;

  x0 = AppCameraDisplay_MaxI32(x0, 0);
  y0 = AppCameraDisplay_MaxI32(y0, 0);
  x1 = AppCameraDisplay_MinI32(x1, (int32_t)APP_CAMERA_DISPLAY_WIDTH);
  y1 = AppCameraDisplay_MinI32(y1, (int32_t)APP_CAMERA_DISPLAY_HEIGHT);

  if ((frame_addr == 0U) || (x1 <= x0) || (y1 <= y0))
  {
    return;
  }

  width = (uint32_t)(x1 - x0);
  height = (uint32_t)(y1 - y0);
  address = frame_addr +
            ((uint32_t)y0 * APP_CAMERA_DISPLAY_CAMERA_LINE_BYTES) +
            ((uint32_t)x0 * APP_CAMERA_DISPLAY_BYTES_PER_PIXEL);
  bytes = (((height - 1U) * APP_CAMERA_DISPLAY_CAMERA_LINE_BYTES) +
           (width * APP_CAMERA_DISPLAY_BYTES_PER_PIXEL));

  if (invalidate != 0U)
  {
    AppCameraDisplay_CleanInvalidateDCache(address, bytes);
  }
  else
  {
    AppCameraDisplay_CleanDCache(address, bytes);
  }
}

static uint16_t AppCameraDisplay_Rgb565(uint8_t r, uint8_t g, uint8_t b)
{
  return (uint16_t)((((uint16_t)r & 0xF8U) << 8) |
                    (((uint16_t)g & 0xFCU) << 3) |
                    ((uint16_t)b >> 3));
}

static uint16_t AppCameraDisplay_HeatColor(uint8_t value)
{
  if (value < 96U)
  {
    const uint32_t t = (uint32_t)value * 255U / 95U;
    return AppCameraDisplay_Rgb565(0U, (uint8_t)(80U + ((t * 120U) / 255U)), (uint8_t)(180U - ((t * 80U) / 255U)));
  }
  if (value < 176U)
  {
    const uint32_t t = ((uint32_t)value - 96U) * 255U / 79U;
    return AppCameraDisplay_Rgb565((uint8_t)((t * 230U) / 255U), (uint8_t)(200U - ((t * 30U) / 255U)), 40U);
  }

  {
    const uint32_t t = ((uint32_t)value - 176U) * 255U / 79U;
    return AppCameraDisplay_Rgb565(230U, (uint8_t)(170U - ((t * 140U) / 255U)), 40U);
  }
}

static uint16_t AppCameraDisplay_BlendRgb565(uint16_t dst, uint16_t src, uint8_t alpha)
{
  const uint32_t inv = 255U - (uint32_t)alpha;
  const uint32_t dr = (dst >> 11) & 0x1FU;
  const uint32_t dg = (dst >> 5) & 0x3FU;
  const uint32_t db = dst & 0x1FU;
  const uint32_t sr = (src >> 11) & 0x1FU;
  const uint32_t sg = (src >> 5) & 0x3FU;
  const uint32_t sb = src & 0x1FU;
  const uint32_t r = ((dr * inv) + (sr * (uint32_t)alpha)) / 255U;
  const uint32_t g = ((dg * inv) + (sg * (uint32_t)alpha)) / 255U;
  const uint32_t b = ((db * inv) + (sb * (uint32_t)alpha)) / 255U;

  return (uint16_t)((r << 11) | (g << 5) | b);
}

static void AppCameraDisplay_DrawBlendRect(uint16_t *framebuffer,
                                           int32_t x,
                                           int32_t y,
                                           int32_t width,
                                           int32_t height,
                                           uint16_t color,
                                           uint8_t alpha)
{
  int32_t x0 = AppCameraDisplay_MaxI32(x, 0);
  int32_t y0 = AppCameraDisplay_MaxI32(y, 0);
  int32_t x1 = AppCameraDisplay_MinI32(x + width, (int32_t)APP_CAMERA_DISPLAY_WIDTH);
  int32_t y1 = AppCameraDisplay_MinI32(y + height, (int32_t)APP_CAMERA_DISPLAY_HEIGHT);

  if ((framebuffer == 0) || (x1 <= x0) || (y1 <= y0))
  {
    return;
  }

  for (int32_t yy = y0; yy < y1; ++yy)
  {
    uint16_t *pixel = framebuffer + ((uint32_t)yy * APP_CAMERA_DISPLAY_WIDTH) + (uint32_t)x0;
    for (int32_t xx = x0; xx < x1; ++xx)
    {
      *pixel = AppCameraDisplay_BlendRgb565(*pixel, color, alpha);
      pixel++;
    }
  }
}

static void AppCameraDisplay_DrawSolidRect(uint16_t *framebuffer,
                                           int32_t x,
                                           int32_t y,
                                           int32_t width,
                                           int32_t height,
                                           uint16_t color)
{
  int32_t x0 = AppCameraDisplay_MaxI32(x, 0);
  int32_t y0 = AppCameraDisplay_MaxI32(y, 0);
  int32_t x1 = AppCameraDisplay_MinI32(x + width, (int32_t)APP_CAMERA_DISPLAY_WIDTH);
  int32_t y1 = AppCameraDisplay_MinI32(y + height, (int32_t)APP_CAMERA_DISPLAY_HEIGHT);

  if ((framebuffer == 0) || (x1 <= x0) || (y1 <= y0))
  {
    return;
  }

  for (int32_t yy = y0; yy < y1; ++yy)
  {
    uint16_t *pixel = framebuffer + ((uint32_t)yy * APP_CAMERA_DISPLAY_WIDTH) + (uint32_t)x0;
    for (int32_t xx = x0; xx < x1; ++xx)
    {
      *pixel++ = color;
    }
  }
}

static uint8_t AppCameraDisplay_OverlayRect(uint8_t value,
                                            uint32_t row,
                                            uint32_t col,
                                            int32_t *x,
                                            int32_t *y,
                                            int32_t *width,
                                            int32_t *height)
{
  const int32_t cell_w = (int32_t)(APP_CAMERA_DISPLAY_WIDTH / APP_CAMERA_DISPLAY_HEAT_GRID_SIZE);
  const int32_t cell_h = (int32_t)(APP_CAMERA_DISPLAY_HEIGHT / APP_CAMERA_DISPLAY_HEAT_GRID_SIZE);
  int32_t marker_w;
  int32_t marker_h;
  int32_t cell_x;
  int32_t cell_y;

  if ((x == 0) || (y == 0) || (width == 0) || (height == 0) ||
      (value < APP_CAMERA_DISPLAY_HEAT_MIN_VALUE))
  {
    return 0U;
  }

  marker_w = 8 + (int32_t)(((uint32_t)value * (uint32_t)(cell_w / 2)) / 255U);
  marker_h = 8 + (int32_t)(((uint32_t)value * (uint32_t)(cell_h / 2)) / 255U);
  cell_x = (int32_t)col * cell_w;
  cell_y = (int32_t)row * cell_h;

  *x = cell_x + ((cell_w - marker_w) / 2);
  *y = cell_y + ((cell_h - marker_h) / 2);
  *width = marker_w;
  *height = marker_h;

  return 1U;
}

static uint8_t AppCameraDisplay_OverlayCellBlock(uint32_t index,
                                                 int32_t *x,
                                                 int32_t *y,
                                                 int32_t *width,
                                                 int32_t *height)
{
  const int32_t cell_w = (int32_t)(APP_CAMERA_DISPLAY_WIDTH / APP_CAMERA_DISPLAY_HEAT_GRID_SIZE);
  const int32_t cell_h = (int32_t)(APP_CAMERA_DISPLAY_HEIGHT / APP_CAMERA_DISPLAY_HEAT_GRID_SIZE);
  const uint32_t row = index / APP_CAMERA_DISPLAY_HEAT_GRID_SIZE;
  const uint32_t col = index % APP_CAMERA_DISPLAY_HEAT_GRID_SIZE;

  if ((index >= APP_CAMERA_DISPLAY_HEAT_CELL_COUNT) ||
      (x == 0) ||
      (y == 0) ||
      (width == 0) ||
      (height == 0))
  {
    return 0U;
  }

  *x = ((int32_t)col * cell_w) + 6;
  *y = ((int32_t)row * cell_h) + 5;
  *width = cell_w - 12;
  *height = cell_h - 10;
  return 1U;
}

static uint8_t AppCameraDisplay_EffectiveHeatValue(const AppCameraDisplayAcousticOverlay_t *overlay,
                                                   uint32_t index)
{
  uint8_t value;

  if ((overlay == 0) || (index >= APP_CAMERA_DISPLAY_HEAT_CELL_COUNT))
  {
    return 0U;
  }

  value = overlay->heat[index];
#if APP_CAMERA_DISPLAY_OVERLAY_BRINGUP_ENABLE
  if ((overlay->enabled != 0U) &&
      (overlay->valid != 0U) &&
      (index == (uint32_t)overlay->peak_index) &&
      (value < APP_CAMERA_DISPLAY_BRINGUP_PEAK_VALUE))
  {
    value = APP_CAMERA_DISPLAY_BRINGUP_PEAK_VALUE;
  }
#endif

  return value;
}

static void AppCameraDisplay_DrawAcousticOverlay(uint32_t frame_addr)
{
  AppCameraDisplayAcousticOverlay_t overlay;
  uint16_t *framebuffer;
  int32_t union_x0 = (int32_t)APP_CAMERA_DISPLAY_WIDTH;
  int32_t union_y0 = (int32_t)APP_CAMERA_DISPLAY_HEIGHT;
  int32_t union_x1 = 0;
  int32_t union_y1 = 0;
  uint8_t has_rect = 0U;
  uint8_t alpha_base;

  overlay = s_acoustic_overlay;
  if ((overlay.enabled == 0U) || (overlay.valid == 0U) || (frame_addr == 0U))
  {
    return;
  }

  for (uint32_t row = 0U; row < APP_CAMERA_DISPLAY_HEAT_GRID_SIZE; ++row)
  {
    for (uint32_t col = 0U; col < APP_CAMERA_DISPLAY_HEAT_GRID_SIZE; ++col)
    {
      int32_t x;
      int32_t y;
      int32_t w;
      int32_t h;
      const uint32_t index = (row * APP_CAMERA_DISPLAY_HEAT_GRID_SIZE) + col;

      const uint8_t value = AppCameraDisplay_EffectiveHeatValue(&overlay, index);

      if (AppCameraDisplay_OverlayRect(value, row, col, &x, &y, &w, &h) != 0U)
      {
        union_x0 = AppCameraDisplay_MinI32(union_x0, x);
        union_y0 = AppCameraDisplay_MinI32(union_y0, y);
        union_x1 = AppCameraDisplay_MaxI32(union_x1, x + w);
        union_y1 = AppCameraDisplay_MaxI32(union_y1, y + h);
        has_rect = 1U;
      }
    }
  }

#if APP_CAMERA_DISPLAY_OVERLAY_BRINGUP_ENABLE
  if (overlay.peak_index < APP_CAMERA_DISPLAY_HEAT_CELL_COUNT)
  {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;

    if (AppCameraDisplay_OverlayCellBlock(overlay.peak_index, &x, &y, &w, &h) != 0U)
    {
      union_x0 = AppCameraDisplay_MinI32(union_x0, x);
      union_y0 = AppCameraDisplay_MinI32(union_y0, y);
      union_x1 = AppCameraDisplay_MaxI32(union_x1, x + w);
      union_y1 = AppCameraDisplay_MaxI32(union_y1, y + h);
      has_rect = 1U;
    }
  }
#endif

  if (has_rect == 0U)
  {
    return;
  }

  union_x0 = AppCameraDisplay_MaxI32(union_x0 - 3, 0);
  union_y0 = AppCameraDisplay_MaxI32(union_y0 - 3, 0);
  union_x1 = AppCameraDisplay_MinI32(union_x1 + 3, (int32_t)APP_CAMERA_DISPLAY_WIDTH);
  union_y1 = AppCameraDisplay_MinI32(union_y1 + 3, (int32_t)APP_CAMERA_DISPLAY_HEIGHT);

  AppCameraDisplay_FlushCameraRect(frame_addr, union_x0, union_y0, union_x1, union_y1, 1U);

  framebuffer = (uint16_t *)frame_addr;
  alpha_base = (overlay.quality_pct < 72U) ? 72U : overlay.quality_pct;
  for (uint32_t row = 0U; row < APP_CAMERA_DISPLAY_HEAT_GRID_SIZE; ++row)
  {
    for (uint32_t col = 0U; col < APP_CAMERA_DISPLAY_HEAT_GRID_SIZE; ++col)
    {
      int32_t x;
      int32_t y;
      int32_t w;
      int32_t h;
      uint8_t alpha;
      const uint32_t index = (row * APP_CAMERA_DISPLAY_HEAT_GRID_SIZE) + col;
      const uint8_t value = AppCameraDisplay_EffectiveHeatValue(&overlay, index);

      if (AppCameraDisplay_OverlayRect(value, row, col, &x, &y, &w, &h) == 0U)
      {
        continue;
      }

      alpha = (uint8_t)(96U + (((uint32_t)value * (uint32_t)(96U + alpha_base)) / 510U));
      AppCameraDisplay_DrawBlendRect(framebuffer,
                                     x,
                                     y,
                                     w,
                                     h,
                                     AppCameraDisplay_HeatColor(value),
                                     alpha);
    }
  }

  if (overlay.peak_index < APP_CAMERA_DISPLAY_HEAT_CELL_COUNT)
  {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
    const uint32_t peak_row = overlay.peak_index / APP_CAMERA_DISPLAY_HEAT_GRID_SIZE;
    const uint32_t peak_col = overlay.peak_index % APP_CAMERA_DISPLAY_HEAT_GRID_SIZE;
    const uint8_t peak_value = AppCameraDisplay_EffectiveHeatValue(&overlay, overlay.peak_index);

    if (AppCameraDisplay_OverlayRect(peak_value, peak_row, peak_col, &x, &y, &w, &h) != 0U)
    {
      const uint16_t white = AppCameraDisplay_Rgb565(245U, 246U, 238U);
      AppCameraDisplay_DrawSolidRect(framebuffer, x, y, w, 4, white);
      AppCameraDisplay_DrawSolidRect(framebuffer, x, y + h - 4, w, 4, white);
      AppCameraDisplay_DrawSolidRect(framebuffer, x, y, 4, h, white);
      AppCameraDisplay_DrawSolidRect(framebuffer, x + w - 4, y, 4, h, white);
    }
  }

#if APP_CAMERA_DISPLAY_OVERLAY_BRINGUP_ENABLE
  if (overlay.peak_index < APP_CAMERA_DISPLAY_HEAT_CELL_COUNT)
  {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;

    if (AppCameraDisplay_OverlayCellBlock(overlay.peak_index, &x, &y, &w, &h) != 0U)
    {
      const uint16_t peak_fill = AppCameraDisplay_HeatColor(APP_CAMERA_DISPLAY_BRINGUP_PEAK_VALUE);
      const uint16_t peak_border = AppCameraDisplay_Rgb565(255U, 245U, 170U);
      AppCameraDisplay_DrawBlendRect(framebuffer, x, y, w, h, peak_fill, 196U);
      AppCameraDisplay_DrawSolidRect(framebuffer, x, y, w, 5, peak_border);
      AppCameraDisplay_DrawSolidRect(framebuffer, x, y + h - 5, w, 5, peak_border);
      AppCameraDisplay_DrawSolidRect(framebuffer, x, y, 5, h, peak_border);
      AppCameraDisplay_DrawSolidRect(framebuffer, x + w - 5, y, 5, h, peak_border);
    }
  }
#endif

  AppCameraDisplay_FlushCameraRect(frame_addr, union_x0, union_y0, union_x1, union_y1, 0U);
  g_app_camera_overlay_draw_count++;
}

static void AppCameraDisplay_SnapshotLtdc(void)
{
  g_app_camera_ltdc_ier2 = LTDC->IER2;
  g_app_camera_ltdc_isr2 = LTDC->ISR2;
  g_app_camera_ltdc_layer1_cr = LTDC_Layer1->CR;
  g_app_camera_ltdc_layer1_cfbar = LTDC_Layer1->CFBAR;
  g_app_camera_ltdc_layer2_cr = LTDC_Layer2->CR;
  g_app_camera_ltdc_layer2_cfbar = LTDC_Layer2->CFBAR;
  g_app_camera_ui_fb_addr = AppCameraDisplay_GetUiFramebufferAddr();
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
  uint32_t ui_fb_addr = AppCameraDisplay_GetUiFramebufferAddr();

  AppCameraDisplay_FillRgb565Layer(&layer,
                                   0U,
                                   0U,
                                   APP_CAMERA_DISPLAY_SCREEN_WIDTH,
                                   APP_CAMERA_DISPLAY_SCREEN_HEIGHT,
                                   ui_fb_addr);
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

static uint32_t AppCameraDisplay_PrepareDisplayFrame(uint32_t frame_addr)
{
#if APP_CAMERA_DISPLAY_COMPOSE_ENABLE
  const uint32_t compose_addr = (uint32_t)s_camera_display_compose_buffer;

  if (frame_addr == 0U)
  {
    return 0U;
  }

  AppCameraDisplay_InvalidateDCache(frame_addr, APP_CAMERA_DISPLAY_CAMERA_FRAME_BYTES);
  (void)memcpy((void *)compose_addr,
               (const void *)frame_addr,
               APP_CAMERA_DISPLAY_CAMERA_FRAME_BYTES);
  AppCameraDisplay_CleanDCache(compose_addr, APP_CAMERA_DISPLAY_CAMERA_FRAME_BYTES);
  g_app_camera_compose_addr = compose_addr;
  g_app_camera_compose_count++;
  return compose_addr;
#else
  return frame_addr;
#endif
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

void AppCameraDisplay_SetAcousticOverlay(const uint8_t *heat,
                                         uint32_t count,
                                         uint8_t peak_index,
                                         uint8_t quality_pct,
                                         uint8_t enabled)
{
  uint32_t primask;

  primask = __get_PRIMASK();
  __disable_irq();

  s_acoustic_overlay.enabled = (enabled != 0U) ? 1U : 0U;
  s_acoustic_overlay.valid = ((enabled != 0U) && (heat != 0) && (count != 0U)) ? 1U : 0U;
  s_acoustic_overlay.peak_index = (peak_index < APP_CAMERA_DISPLAY_HEAT_CELL_COUNT) ? peak_index : 0U;
  s_acoustic_overlay.quality_pct = (quality_pct > 100U) ? 100U : quality_pct;

  for (uint32_t i = 0U; i < APP_CAMERA_DISPLAY_HEAT_CELL_COUNT; ++i)
  {
    s_acoustic_overlay.heat[i] = ((heat != 0) && (i < count)) ? heat[i] : 0U;
  }

  if (primask == 0U)
  {
    __enable_irq();
  }

  g_app_camera_overlay_update_count++;
}

void AppCameraDisplay_RefreshColorKeyHole(int32_t x,
                                          int32_t y,
                                          int32_t width,
                                          int32_t height)
{
  const int32_t hole_x0 = (int32_t)APP_CAMERA_DISPLAY_X0;
  const int32_t hole_y0 =
    (int32_t)APP_CAMERA_DISPLAY_Y0;
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

  framebuffer = (uint16_t *)AppCameraDisplay_GetUiFramebufferAddr();
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
  uint32_t display_addr;

  if (((g_app_camera_display_flags & APP_CAMERA_DISPLAY_FLAG_READY) == 0U) || (frame_addr == 0U))
  {
    return;
  }

  AppCameraDisplay_SnapshotLtdc();
  if ((g_app_camera_ltdc_isr2 & APP_CAMERA_DISPLAY_LTDC_ERROR_MASK) != 0U)
  {
    g_app_camera_ltdc_error_count++;
    AppCameraDisplay_ClearLtdcErrors();
  }

  display_addr = AppCameraDisplay_PrepareDisplayFrame(frame_addr);
  if (display_addr == 0U)
  {
    return;
  }

  g_app_camera_pending_display_addr = display_addr;
  g_app_camera_display_addr = display_addr;
  if ((g_app_camera_display_flags & APP_CAMERA_DISPLAY_FLAG_VISIBLE) != 0U)
  {
    AppCameraDisplay_DrawAcousticOverlay(display_addr);
    LTDC_Layer1->CFBAR = display_addr;
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
    status->ltdc_layer1_cr = g_app_camera_ltdc_layer1_cr;
    status->ltdc_layer1_cfbar = g_app_camera_ltdc_layer1_cfbar;
    status->ltdc_layer2_cr = g_app_camera_ltdc_layer2_cr;
    status->ltdc_layer2_cfbar = g_app_camera_ltdc_layer2_cfbar;
    status->ui_fb_addr = g_app_camera_ui_fb_addr;
    status->auto_disable_count = g_app_camera_ltdc_auto_disable_count;
    status->overlay_update_count = g_app_camera_overlay_update_count;
    status->overlay_draw_count = g_app_camera_overlay_draw_count;
    status->init_status = (int32_t)g_app_camera_display_init_status;
  }
}
