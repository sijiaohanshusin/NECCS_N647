#include "app_camera_display.h"

#include "app_camera.h"
#include "main.h"
#include <string.h>

#define APP_CAMERA_DISPLAY_FLAG_READY (1UL << 0)
#define APP_CAMERA_DISPLAY_FLAG_VISIBLE (1UL << 1)
#define APP_CAMERA_DISPLAY_LTDC_ERROR_MASK \
  (LTDC_ISR2_FUWIF | LTDC_ISR2_TERRIF | LTDC_ISR2_FUIF | LTDC_ISR2_CRCIF)
#define APP_CAMERA_DISPLAY_DCACHE_LINE_BYTES 32U
#define APP_CAMERA_DISPLAY_COMPOSE_ENABLE          1U
#define APP_CAMERA_DISPLAY_DMA2D_COPY_ENABLE       1U
#define APP_CAMERA_DISPLAY_DMA2D_TIMEOUT_MS        10U

/* Heat overlay rendering: normalized values below this stay fully
 * transparent; above, alpha ramps up to the (quality-scaled) maximum. */
#define APP_CAMERA_DISPLAY_HEAT_ALPHA_MIN_VALUE 18U
#define APP_CAMERA_DISPLAY_HEAT_ALPHA_BASE      56U
#define APP_CAMERA_DISPLAY_HEAT_ALPHA_MAX       208U
#define APP_CAMERA_DISPLAY_MARKER_MIN_STRENGTH  64U

extern LTDC_HandleTypeDef hltdc;
extern DMA2D_HandleTypeDef hdma2d;

typedef struct
{
  uint8_t enabled;
  uint8_t quality_pct;
  uint8_t marker_count;
  AppCameraDisplayMarker_t markers[APP_CAMERA_DISPLAY_MARKER_MAX];
  uint8_t field[APP_CAMERA_DISPLAY_FIELD_COUNT];
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
volatile uint32_t g_app_camera_overlay_draw_cycles = 0U;
volatile uint32_t g_app_camera_compose_addr = 0U;
volatile uint32_t g_app_camera_compose_count = 0U;
volatile uint32_t g_app_camera_dma2d_copy_count = 0U;
volatile uint32_t g_app_camera_dma2d_fallback_count = 0U;
volatile uint32_t g_app_camera_dma2d_error_code = 0U;

/* CPU-only shared state; cached external RAM keeps internal SRAM for code.
 * Never read before first write (Set/Draw guard on the enabled flag). */
static AppCameraDisplayAcousticOverlay_t s_acoustic_overlay
  __attribute__((section(".EXTRAM"), aligned(32)));
static uint16_t s_camera_display_compose_buffer[APP_CAMERA_DISPLAY_WIDTH * APP_CAMERA_DISPLAY_HEIGHT]
  __attribute__((section(".EXTRAM"), aligned(32)));

/* Heat palettes (256-entry RGB565 LUTs) + fixed-point bilinear sample maps.
 * Built at runtime (never read before s_heat_luts_ready), CPU-only and small
 * enough to live in D-cache, so cached external RAM is fine. */
static uint16_t s_heat_palette_lut[3][256] __attribute__((section(".EXTRAM"), aligned(32)));
static uint8_t s_heat_alpha_lut[256] __attribute__((section(".EXTRAM"), aligned(32)));
static uint16_t s_heat_u0[APP_CAMERA_DISPLAY_WIDTH / 2U] __attribute__((section(".EXTRAM"), aligned(32)));
static uint8_t s_heat_wu[APP_CAMERA_DISPLAY_WIDTH / 2U] __attribute__((section(".EXTRAM"), aligned(32)));
static uint16_t s_heat_v0[APP_CAMERA_DISPLAY_HEIGHT / 2U] __attribute__((section(".EXTRAM"), aligned(32)));
static uint8_t s_heat_wv[APP_CAMERA_DISPLAY_HEIGHT / 2U] __attribute__((section(".EXTRAM"), aligned(32)));
static volatile uint8_t s_heat_palette_index = 0U;
static uint8_t s_heat_luts_ready = 0U;

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

/* Piecewise-linear palette stops (value 0..255 -> RGB888). */
typedef struct
{
  uint8_t position;
  uint8_t r;
  uint8_t g;
  uint8_t b;
} AppCameraDisplayPaletteStop_t;

static const AppCameraDisplayPaletteStop_t s_palette_iron[] =
{
  {   0,   8,   4,  32 },
  {  64,  64,   8, 116 },
  { 128, 196,  48,  28 },
  { 192, 252, 152,  16 },
  { 255, 255, 244, 180 },
};

static const AppCameraDisplayPaletteStop_t s_palette_rainbow[] =
{
  {   0,  12,  24, 148 },
  {  72,   0, 168, 216 },
  { 128,  32, 200,  96 },
  { 192, 244, 212,  40 },
  { 255, 236,  48,  36 },
};

static const AppCameraDisplayPaletteStop_t s_palette_contrast[] =
{
  {   0,  10,  22,  48 },
  { 112,  24, 108, 220 },
  { 200,  92, 220, 255 },
  { 255, 250, 252, 255 },
};

static void AppCameraDisplay_BuildPaletteLut(uint16_t *lut,
                                             const AppCameraDisplayPaletteStop_t *stops,
                                             uint32_t stop_count)
{
  uint32_t segment = 0U;

  for (uint32_t v = 0U; v < 256U; v++)
  {
    while ((segment < (stop_count - 2U)) && (v > (uint32_t)stops[segment + 1U].position))
    {
      segment++;
    }

    {
      const AppCameraDisplayPaletteStop_t *a = &stops[segment];
      const AppCameraDisplayPaletteStop_t *b = &stops[segment + 1U];
      const uint32_t span = (uint32_t)b->position - (uint32_t)a->position;
      const uint32_t t = (span != 0U) ? (((v - (uint32_t)a->position) * 255U) / span) : 0U;
      const uint8_t r = (uint8_t)(a->r + (((int32_t)b->r - (int32_t)a->r) * (int32_t)t) / 255);
      const uint8_t g = (uint8_t)(a->g + (((int32_t)b->g - (int32_t)a->g) * (int32_t)t) / 255);
      const uint8_t bl = (uint8_t)(a->b + (((int32_t)b->b - (int32_t)a->b) * (int32_t)t) / 255);

      lut[v] = AppCameraDisplay_Rgb565(r, g, bl);
    }
  }
}

static void AppCameraDisplay_BuildHeatLuts(void)
{
  if (s_heat_luts_ready != 0U)
  {
    return;
  }

  AppCameraDisplay_BuildPaletteLut(s_heat_palette_lut[0],
                                   s_palette_iron,
                                   sizeof(s_palette_iron) / sizeof(s_palette_iron[0]));
  AppCameraDisplay_BuildPaletteLut(s_heat_palette_lut[1],
                                   s_palette_rainbow,
                                   sizeof(s_palette_rainbow) / sizeof(s_palette_rainbow[0]));
  AppCameraDisplay_BuildPaletteLut(s_heat_palette_lut[2],
                                   s_palette_contrast,
                                   sizeof(s_palette_contrast) / sizeof(s_palette_contrast[0]));

  for (uint32_t v = 0U; v < 256U; v++)
  {
    if (v < APP_CAMERA_DISPLAY_HEAT_ALPHA_MIN_VALUE)
    {
      s_heat_alpha_lut[v] = 0U;
    }
    else
    {
      const uint32_t span = 255U - APP_CAMERA_DISPLAY_HEAT_ALPHA_MIN_VALUE;
      const uint32_t t = ((v - APP_CAMERA_DISPLAY_HEAT_ALPHA_MIN_VALUE) * 255U) / span;
      s_heat_alpha_lut[v] = (uint8_t)(APP_CAMERA_DISPLAY_HEAT_ALPHA_BASE +
                                      ((t * (APP_CAMERA_DISPLAY_HEAT_ALPHA_MAX -
                                             APP_CAMERA_DISPLAY_HEAT_ALPHA_BASE)) / 255U));
    }
  }

  /* Fixed-point source coordinates for the 2x2-block bilinear upscale. */
  for (uint32_t bx = 0U; bx < (APP_CAMERA_DISPLAY_WIDTH / 2U); bx++)
  {
    const float x = ((float)(bx * 2U) + 1.0f);
    float u = (x * (float)APP_CAMERA_DISPLAY_FIELD_W / (float)APP_CAMERA_DISPLAY_WIDTH) - 0.5f;
    uint32_t u0;

    if (u < 0.0f)
    {
      u = 0.0f;
    }
    u0 = (uint32_t)u;
    if (u0 >= (APP_CAMERA_DISPLAY_FIELD_W - 1U))
    {
      u0 = APP_CAMERA_DISPLAY_FIELD_W - 2U;
    }
    s_heat_u0[bx] = (uint16_t)u0;
    s_heat_wu[bx] = (uint8_t)((u - (float)u0) * 255.0f);
  }
  for (uint32_t by = 0U; by < (APP_CAMERA_DISPLAY_HEIGHT / 2U); by++)
  {
    const float y = ((float)(by * 2U) + 1.0f);
    float v = (y * (float)APP_CAMERA_DISPLAY_FIELD_H / (float)APP_CAMERA_DISPLAY_HEIGHT) - 0.5f;
    uint32_t v0;

    if (v < 0.0f)
    {
      v = 0.0f;
    }
    v0 = (uint32_t)v;
    if (v0 >= (APP_CAMERA_DISPLAY_FIELD_H - 1U))
    {
      v0 = APP_CAMERA_DISPLAY_FIELD_H - 2U;
    }
    s_heat_v0[by] = (uint16_t)v0;
    s_heat_wv[by] = (uint8_t)((v - (float)v0) * 255.0f);
  }

  s_heat_luts_ready = 1U;
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

static void AppCameraDisplay_DrawMarker(uint16_t *framebuffer,
                                        const AppCameraDisplayMarker_t *marker,
                                        uint8_t is_primary)
{
  const int32_t x = (int32_t)marker->x;
  const int32_t y = (int32_t)marker->y;
  const int32_t arm = is_primary ? 14 : 9;
  const int32_t gap = is_primary ? 4 : 3;
  const int32_t thick = is_primary ? 2 : 2;
  const uint16_t color = is_primary ? AppCameraDisplay_Rgb565(245U, 246U, 240U)
                                    : AppCameraDisplay_Rgb565(120U, 214U, 255U);

  AppCameraDisplay_DrawSolidRect(framebuffer, x - arm, y - (thick / 2), arm - gap, thick, color);
  AppCameraDisplay_DrawSolidRect(framebuffer, x + gap, y - (thick / 2), arm - gap, thick, color);
  AppCameraDisplay_DrawSolidRect(framebuffer, x - (thick / 2), y - arm, thick, arm - gap, color);
  AppCameraDisplay_DrawSolidRect(framebuffer, x - (thick / 2), y + gap, thick, arm - gap, color);

  if (is_primary != 0U)
  {
    const int32_t box = 24;
    AppCameraDisplay_DrawSolidRect(framebuffer, x - box, y - box, 10, 2, color);
    AppCameraDisplay_DrawSolidRect(framebuffer, x - box, y - box, 2, 10, color);
    AppCameraDisplay_DrawSolidRect(framebuffer, x + box - 10, y - box, 10, 2, color);
    AppCameraDisplay_DrawSolidRect(framebuffer, x + box - 2, y - box, 2, 10, color);
    AppCameraDisplay_DrawSolidRect(framebuffer, x - box, y + box - 2, 10, 2, color);
    AppCameraDisplay_DrawSolidRect(framebuffer, x - box, y + box - 10, 2, 10, color);
    AppCameraDisplay_DrawSolidRect(framebuffer, x + box - 10, y + box - 2, 10, 2, color);
    AppCameraDisplay_DrawSolidRect(framebuffer, x + box - 2, y + box - 10, 2, 10, color);
  }
}

/* Render the 48x36 heat field over the camera frame: fixed-point bilinear
 * upscale in 2x2 pixel blocks, palette LUT colouring and per-pixel alpha.
 * Runs on the camera thread at swap rate; measured via DWT into
 * g_app_camera_overlay_draw_cycles. */
static void AppCameraDisplay_DrawAcousticOverlay(uint32_t frame_addr)
{
  AppCameraDisplayAcousticOverlay_t overlay;
  uint16_t *framebuffer;
  const uint16_t *palette;
  uint32_t qscale;
  uint32_t cycles_start;
  uint32_t primask;

  primask = __get_PRIMASK();
  __disable_irq();
  overlay = s_acoustic_overlay;
  if (primask == 0U)
  {
    __enable_irq();
  }

  if ((overlay.enabled == 0U) || (frame_addr == 0U))
  {
    return;
  }

  AppCameraDisplay_BuildHeatLuts();
  cycles_start = DWT->CYCCNT;

  palette = s_heat_palette_lut[(s_heat_palette_index < 3U) ? s_heat_palette_index : 0U];

  /* Alpha confidence scaling: quality 0..100 -> qscale 168..255. */
  qscale = 168U + (((uint32_t)overlay.quality_pct * 87U) / 100U);

  AppCameraDisplay_FlushCameraRect(frame_addr,
                                   0,
                                   0,
                                   (int32_t)APP_CAMERA_DISPLAY_WIDTH,
                                   (int32_t)APP_CAMERA_DISPLAY_HEIGHT,
                                   1U);

  framebuffer = (uint16_t *)frame_addr;

  for (uint32_t by = 0U; by < (APP_CAMERA_DISPLAY_HEIGHT / 2U); by++)
  {
    const uint32_t v0 = s_heat_v0[by];
    const uint32_t wv = s_heat_wv[by];
    const uint32_t inv_wv = 255U - wv;
    const uint8_t *row0 = &overlay.field[v0 * APP_CAMERA_DISPLAY_FIELD_W];
    const uint8_t *row1 = row0 + APP_CAMERA_DISPLAY_FIELD_W;
    uint16_t *dst0 = framebuffer + ((by * 2U) * APP_CAMERA_DISPLAY_WIDTH);
    uint16_t *dst1 = dst0 + APP_CAMERA_DISPLAY_WIDTH;

    for (uint32_t bx = 0U; bx < (APP_CAMERA_DISPLAY_WIDTH / 2U); bx++)
    {
      const uint32_t u0 = s_heat_u0[bx];
      const uint32_t wu = s_heat_wu[bx];
      const uint32_t inv_wu = 255U - wu;
      uint32_t top;
      uint32_t bottom;
      uint32_t value;
      uint32_t alpha;

      top = ((uint32_t)row0[u0] * inv_wu) + ((uint32_t)row0[u0 + 1U] * wu);
      bottom = ((uint32_t)row1[u0] * inv_wu) + ((uint32_t)row1[u0 + 1U] * wu);
      value = ((top * inv_wv) + (bottom * wv)) >> 16;

      alpha = s_heat_alpha_lut[value & 0xFFU];
      if (alpha == 0U)
      {
        dst0 += 2;
        dst1 += 2;
        continue;
      }

      alpha = (alpha * qscale) >> 8;

      {
        const uint16_t color = palette[value & 0xFFU];
        const uint32_t x = bx * 2U;

        dst0[0] = AppCameraDisplay_BlendRgb565(dst0[0], color, (uint8_t)alpha);
        dst0[1] = AppCameraDisplay_BlendRgb565(dst0[1], color, (uint8_t)alpha);
        dst1[0] = AppCameraDisplay_BlendRgb565(dst1[0], color, (uint8_t)alpha);
        dst1[1] = AppCameraDisplay_BlendRgb565(dst1[1], color, (uint8_t)alpha);
        (void)x;
      }

      dst0 += 2;
      dst1 += 2;
    }
  }

  for (uint32_t i = 0U; i < overlay.marker_count; i++)
  {
    if (overlay.markers[i].strength >= APP_CAMERA_DISPLAY_MARKER_MIN_STRENGTH)
    {
      AppCameraDisplay_DrawMarker(framebuffer, &overlay.markers[i], (i == 0U) ? 1U : 0U);
    }
  }

  AppCameraDisplay_FlushCameraRect(frame_addr,
                                   0,
                                   0,
                                   (int32_t)APP_CAMERA_DISPLAY_WIDTH,
                                   (int32_t)APP_CAMERA_DISPLAY_HEIGHT,
                                   0U);
  g_app_camera_overlay_draw_cycles = DWT->CYCCNT - cycles_start;
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

static uint8_t AppCameraDisplay_CopyFrameDma2d(uint32_t src_addr,
                                               uint32_t dst_addr)
{
#if APP_CAMERA_DISPLAY_DMA2D_COPY_ENABLE
  HAL_StatusTypeDef status;

  if ((src_addr == 0U) ||
      (dst_addr == 0U) ||
      (hdma2d.Instance == 0) ||
      (hdma2d.State != HAL_DMA2D_STATE_READY))
  {
    g_app_camera_dma2d_fallback_count++;
    return 0U;
  }

  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = 0U;
  hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR;
  hdma2d.Init.BytesSwap = DMA2D_BYTES_REGULAR;
  hdma2d.Init.LineOffsetMode = DMA2D_LOM_PIXELS;
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputOffset = 0U;
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputAlpha = 255U;
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].RedBlueSwap = DMA2D_RB_REGULAR;

  AppCameraDisplay_CleanInvalidateDCache(dst_addr, APP_CAMERA_DISPLAY_CAMERA_FRAME_BYTES);

  status = HAL_DMA2D_Init(&hdma2d);
  if (status == HAL_OK)
  {
    status = HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER);
  }
  if (status == HAL_OK)
  {
    status = HAL_DMA2D_Start(&hdma2d,
                             src_addr,
                             dst_addr,
                             APP_CAMERA_DISPLAY_WIDTH,
                             APP_CAMERA_DISPLAY_HEIGHT);
  }
  if (status == HAL_OK)
  {
    status = HAL_DMA2D_PollForTransfer(&hdma2d, APP_CAMERA_DISPLAY_DMA2D_TIMEOUT_MS);
  }
  if (status != HAL_OK)
  {
    g_app_camera_dma2d_error_code = hdma2d.ErrorCode;
    if (hdma2d.State != HAL_DMA2D_STATE_READY)
    {
      (void)HAL_DMA2D_Abort(&hdma2d);
    }
    g_app_camera_dma2d_fallback_count++;
    return 0U;
  }

  AppCameraDisplay_InvalidateDCache(dst_addr, APP_CAMERA_DISPLAY_CAMERA_FRAME_BYTES);
  g_app_camera_dma2d_copy_count++;
  return 1U;
#else
  (void)src_addr;
  (void)dst_addr;
  return 0U;
#endif
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
  if (AppCameraDisplay_CopyFrameDma2d(frame_addr, compose_addr) == 0U)
  {
    (void)memcpy((void *)compose_addr,
                 (const void *)frame_addr,
                 APP_CAMERA_DISPLAY_CAMERA_FRAME_BYTES);
    AppCameraDisplay_CleanDCache(compose_addr, APP_CAMERA_DISPLAY_CAMERA_FRAME_BYTES);
  }
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

  /* Overlay state lives in .EXTRAM which is not zero-initialised by the
   * startup code; sanitize it before any draw can observe it. */
  memset(&s_acoustic_overlay, 0, sizeof(s_acoustic_overlay));

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

void AppCameraDisplay_SetAcousticField(const uint8_t *field,
                                       uint32_t count,
                                       const AppCameraDisplayMarker_t *markers,
                                       uint8_t marker_count,
                                       uint8_t quality_pct,
                                       uint8_t enabled)
{
  uint32_t primask;

  primask = __get_PRIMASK();
  __disable_irq();

  s_acoustic_overlay.enabled = ((enabled != 0U) && (field != 0) &&
                                (count >= APP_CAMERA_DISPLAY_FIELD_COUNT)) ? 1U : 0U;
  s_acoustic_overlay.quality_pct = (quality_pct > 100U) ? 100U : quality_pct;

  if (s_acoustic_overlay.enabled != 0U)
  {
    memcpy(s_acoustic_overlay.field, field, APP_CAMERA_DISPLAY_FIELD_COUNT);
  }
  else
  {
    memset(s_acoustic_overlay.field, 0, sizeof(s_acoustic_overlay.field));
  }

  s_acoustic_overlay.marker_count = 0U;
  if ((markers != 0) && (marker_count != 0U))
  {
    for (uint32_t i = 0U; (i < marker_count) && (i < APP_CAMERA_DISPLAY_MARKER_MAX); i++)
    {
      if ((markers[i].x < APP_CAMERA_DISPLAY_WIDTH) &&
          (markers[i].y < APP_CAMERA_DISPLAY_HEIGHT))
      {
        s_acoustic_overlay.markers[s_acoustic_overlay.marker_count] = markers[i];
        s_acoustic_overlay.marker_count++;
      }
    }
  }

  if (primask == 0U)
  {
    __enable_irq();
  }

  g_app_camera_overlay_update_count++;
}

void AppCameraDisplay_SetHeatPalette(uint8_t palette)
{
  s_heat_palette_index = (palette < 3U) ? palette : 0U;
}

uint8_t AppCameraDisplay_GetHeatPalette(void)
{
  return s_heat_palette_index;
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
    status->dma2d_copy_count = g_app_camera_dma2d_copy_count;
    status->dma2d_fallback_count = g_app_camera_dma2d_fallback_count;
    status->dma2d_error_code = g_app_camera_dma2d_error_code;
    status->init_status = (int32_t)g_app_camera_display_init_status;
  }
}
