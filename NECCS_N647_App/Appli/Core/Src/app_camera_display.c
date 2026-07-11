#include "app_camera_display.h"

#include "app_camera.h"
#include "main.h"
#include "tx_api.h"
#include <string.h>

#define APP_CAMERA_DISPLAY_FLAG_READY (1UL << 0)
#define APP_CAMERA_DISPLAY_FLAG_VISIBLE (1UL << 1)
#define APP_CAMERA_DISPLAY_LTDC_ERROR_MASK \
  (LTDC_ISR2_FUWIF | LTDC_ISR2_TERRIF | LTDC_ISR2_FUIF | LTDC_ISR2_CRCIF)
#define APP_CAMERA_DISPLAY_DCACHE_LINE_BYTES 32U
/* No compose copy: the DCMIPP ping-pongs FB0/FB1 and the LTDC flips to the
 * completed buffer at vertical blanking (VBR reload), so scan-out never sees
 * a partially written frame. The heat overlay draws directly into the
 * completed buffer before the flip. The previous design copied each 614 KB
 * frame into a HyperRAM compose buffer; with DCMIPP + LTDC + GPU2D already
 * saturating HyperRAM, that memcpy measured ~130 ms/frame and starved every
 * lower-priority thread (SRP never finished a single frame). */

/* Heat overlay rendering: normalized values below this stay fully
 * transparent; above, alpha ramps up to the (quality-scaled) maximum.
 * The threshold also bounds the blend cost: every covered pixel is a
 * read-modify-write against HyperRAM (~5 MB/s effective for CPU access),
 * so diffuse low-value fields must stay transparent. */
/* Only the strongest part of the normalised field is drawn (top ~8 dB of
 * the -15 dB window). This matches real acoustic-camera behaviour (heat blob
 * around sources, camera image elsewhere) and, just as importantly, keeps
 * the HyperRAM read-modify-write footprint small: blending the whole
 * 640x480 window against contended HyperRAM measured ~75 ms/frame. */
#define APP_CAMERA_DISPLAY_HEAT_ALPHA_MIN_VALUE 72U
#define APP_CAMERA_DISPLAY_HEAT_ALPHA_BASE      56U
#define APP_CAMERA_DISPLAY_HEAT_ALPHA_MAX       208U
#define APP_CAMERA_DISPLAY_MARKER_MIN_STRENGTH  64U

extern LTDC_HandleTypeDef hltdc;
extern DMA2D_HandleTypeDef hdma2d;

#define APP_CAMERA_DISPLAY_TRAIL_MAX 20U

typedef struct
{
  uint16_t x;
  uint16_t y;
  uint8_t life; /* 0 = free slot; decays on every field update */
} AppCameraDisplayTrailDot_t;

typedef struct
{
  uint8_t enabled;
  uint8_t quality_pct;
  uint8_t marker_count;
  uint8_t trail_enabled;
  AppCameraDisplayMarker_t markers[APP_CAMERA_DISPLAY_MARKER_MAX];
  AppCameraDisplayTrailDot_t trail[APP_CAMERA_DISPLAY_TRAIL_MAX];
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

/* Swap worker: the DCMIPP frame ISR only queues the completed frame address;
 * compose + overlay drawing runs on a low-priority thread so the ISR stays
 * in the microsecond range (a full software overlay pass can take tens of
 * milliseconds and previously starved the SAI/PCMD interrupts). */
#define APP_CAMERA_DISPLAY_SWAP_EVENT 0x1UL
static TX_EVENT_FLAGS_GROUP s_swap_events;
static volatile uint32_t s_swap_pending_addr = 0U;
static volatile uint8_t s_swap_worker_ready = 0U;

/* Overlay state and heat LUTs are the per-pixel hot path of the blend loop.
 * They MUST live in internal SRAM: in external RAM the 614 KB destination
 * stream evicts them from D-cache every frame and each access becomes a
 * HyperRAM round-trip (measured 108 M cycles per overlay draw; ~6 M with
 * the data internal). Total footprint ~8 KB. */
static AppCameraDisplayAcousticOverlay_t s_acoustic_overlay __attribute__((aligned(32)));
static uint16_t s_heat_palette_lut[3][256] __attribute__((aligned(32)));
static uint8_t s_heat_alpha_lut[256] __attribute__((aligned(32)));
static uint16_t s_heat_u0[APP_CAMERA_DISPLAY_WIDTH / 2U] __attribute__((aligned(32)));
static uint8_t s_heat_wu[APP_CAMERA_DISPLAY_WIDTH / 2U] __attribute__((aligned(32)));
static uint16_t s_heat_v0[APP_CAMERA_DISPLAY_HEIGHT / 2U] __attribute__((aligned(32)));
static uint8_t s_heat_wv[APP_CAMERA_DISPLAY_HEIGHT / 2U] __attribute__((aligned(32)));
/* Line-pair staging buffer: the blend must never read-modify-write HyperRAM
 * per pixel (measured ~98 ms/frame that way). Rows are burst-copied here,
 * blended in internal SRAM, then burst-copied back (~2.5 KB). */
static uint16_t s_overlay_linebuf[2][APP_CAMERA_DISPLAY_WIDTH] __attribute__((aligned(32)));
static volatile uint8_t s_heat_palette_index = 0U;
static uint8_t s_heat_luts_ready = 0U;
/* Scanline band touched by the last overlay draw (for targeted cache ops). */
static int32_t s_overlay_dirty_y0 = 0;
static int32_t s_overlay_dirty_y1 = 0;

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

/* Refresh stale lines WITHOUT discarding pending writes: the marker/trail
 * bands overlap rows whose heat-blend results are still dirty in D-cache;
 * a plain invalidate throws those away (horizontal tears through the blob
 * wherever the crosshair box overlaps it). */
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
      /* Quadratic ramp starting at 0 right at the threshold: the hot-region
       * edge fades in instead of stepping to a visible base alpha, which is
       * what made the field read as "blocky" on the panel. */
      const uint32_t span = 255U - APP_CAMERA_DISPLAY_HEAT_ALPHA_MIN_VALUE;
      const uint32_t t = ((v - APP_CAMERA_DISPLAY_HEAT_ALPHA_MIN_VALUE) * 255U) / span;
      const uint32_t t_sq = (t * t) / 255U;
      s_heat_alpha_lut[v] = (uint8_t)((t_sq * APP_CAMERA_DISPLAY_HEAT_ALPHA_MAX) / 255U);
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

/* 5x7 glyphs for the on-frame level label ("-38dB"). */
static const uint8_t *AppCameraDisplay_LevelGlyph(char input)
{
  static const uint8_t g0[7] = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
  static const uint8_t g1[7] = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
  static const uint8_t g2[7] = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
  static const uint8_t g3[7] = {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
  static const uint8_t g4[7] = {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
  static const uint8_t g5[7] = {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E};
  static const uint8_t g6[7] = {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E};
  static const uint8_t g7[7] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
  static const uint8_t g8[7] = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
  static const uint8_t g9[7] = {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C};
  static const uint8_t gminus[7] = {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
  static const uint8_t gd[7] = {0x01, 0x01, 0x0D, 0x13, 0x11, 0x13, 0x0D};
  static const uint8_t gB[7] = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
  static const uint8_t gblank[7] = {0, 0, 0, 0, 0, 0, 0};

  switch (input)
  {
  case '0': return g0;
  case '1': return g1;
  case '2': return g2;
  case '3': return g3;
  case '4': return g4;
  case '5': return g5;
  case '6': return g6;
  case '7': return g7;
  case '8': return g8;
  case '9': return g9;
  case '-': return gminus;
  case 'd': return gd;
  case 'B': return gB;
  default: return gblank;
  }
}

/* Draw text at 2x scale with a 1px dark outline pad for readability. */
static void AppCameraDisplay_DrawLevelText(uint16_t *framebuffer,
                                           int32_t x,
                                           int32_t y,
                                           const char *text)
{
  const uint16_t fg = AppCameraDisplay_Rgb565(245U, 246U, 240U);
  const uint16_t bg = AppCameraDisplay_Rgb565(10U, 16U, 24U);
  const int32_t scale = 2;
  int32_t len = 0;

  while (text[len] != '\0')
  {
    ++len;
  }

  AppCameraDisplay_DrawSolidRect(framebuffer,
                                 x - 3,
                                 y - 3,
                                 (len * 6 * scale) + 4,
                                 (7 * scale) + 6,
                                 bg);

  for (int32_t i = 0; i < len; ++i)
  {
    const uint8_t *rows = AppCameraDisplay_LevelGlyph(text[i]);
    const int32_t gx = x + (i * 6 * scale);

    for (int32_t row = 0; row < 7; ++row)
    {
      for (int32_t col = 0; col < 5; ++col)
      {
        if ((rows[row] & (1U << (4 - col))) != 0U)
        {
          AppCameraDisplay_DrawSolidRect(framebuffer,
                                         gx + (col * scale),
                                         y + (row * scale),
                                         scale,
                                         scale,
                                         fg);
        }
      }
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

    if (marker->level_valid != 0U)
    {
      char text[8];
      int32_t level = (int32_t)marker->level_dbfs;
      uint32_t pos = 0U;

      if (level < 0)
      {
        text[pos++] = '-';
        level = -level;
      }
      if (level >= 10)
      {
        text[pos++] = (char)('0' + (level / 10));
      }
      text[pos++] = (char)('0' + (level % 10));
      text[pos++] = 'd';
      text[pos++] = 'B';
      text[pos] = '\0';

      /* Prefer the right side of the corner box; flip when clipped. */
      {
        const int32_t text_w = ((int32_t)pos * 12) + 4;
        int32_t tx = x + box + 8;
        const int32_t ty = y - box;

        if ((tx + text_w) > (int32_t)APP_CAMERA_DISPLAY_WIDTH)
        {
          tx = x - box - 8 - text_w;
        }
        AppCameraDisplay_DrawLevelText(framebuffer, tx, ty, text);
      }
    }
  }
}

/* Render the 96x72 heat field over the camera frame: fixed-point bilinear
 * upscale in 2x2 pixel blocks, palette LUT colouring and per-pixel alpha.
 * Runs on the camera worker thread at swap rate; measured via DWT into
 * g_app_camera_overlay_draw_cycles. */
static void AppCameraDisplay_DrawAcousticOverlay(uint32_t frame_addr)
{
  /* Static: ~7 KB with the 96x72 field, far too large for the worker thread
   * stack. Only the swap worker calls this function. Kept in internal SRAM
   * (hot per-pixel reads). */
  static AppCameraDisplayAcousticOverlay_t overlay __attribute__((aligned(32)));
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

  framebuffer = (uint16_t *)frame_addr;
  s_overlay_dirty_y0 = (int32_t)APP_CAMERA_DISPLAY_HEIGHT;
  s_overlay_dirty_y1 = 0;

  for (uint32_t by = 0U; by < (APP_CAMERA_DISPLAY_HEIGHT / 2U); by++)
  {
    const uint32_t v0 = s_heat_v0[by];
    const uint32_t wv = s_heat_wv[by];
    const uint32_t inv_wv = 255U - wv;
    const uint8_t *row0 = &overlay.field[v0 * APP_CAMERA_DISPLAY_FIELD_W];
    const uint8_t *row1 = row0 + APP_CAMERA_DISPLAY_FIELD_W;
    uint16_t *dst0 = framebuffer + ((by * 2U) * APP_CAMERA_DISPLAY_WIDTH);
    uint16_t *dst1 = dst0 + APP_CAMERA_DISPLAY_WIDTH;
    uint32_t row_max = 0U;

    /* Skip whole scanline pairs when both source field rows stay below the
     * transparency threshold - the common case for localized sources. */
    for (uint32_t i = 0U; i < APP_CAMERA_DISPLAY_FIELD_W; i++)
    {
      if (row0[i] > row_max)
      {
        row_max = row0[i];
      }
      if (row1[i] > row_max)
      {
        row_max = row1[i];
      }
    }
    if (row_max < APP_CAMERA_DISPLAY_HEAT_ALPHA_MIN_VALUE)
    {
      continue;
    }

    /* Active horizontal span for this line pair: field cells feeding it via
     * the bilinear u-map. Localized sources keep this a small fraction of
     * the width, which directly cuts the HyperRAM traffic. */
    uint32_t cell_min = APP_CAMERA_DISPLAY_FIELD_W;
    uint32_t cell_max = 0U;
    for (uint32_t i = 0U; i < APP_CAMERA_DISPLAY_FIELD_W; i++)
    {
      if ((row0[i] >= APP_CAMERA_DISPLAY_HEAT_ALPHA_MIN_VALUE) ||
          (row1[i] >= APP_CAMERA_DISPLAY_HEAT_ALPHA_MIN_VALUE))
      {
        if (i < cell_min)
        {
          cell_min = i;
        }
        if (i > cell_max)
        {
          cell_max = i;
        }
      }
    }
    if (cell_min > cell_max)
    {
      continue;
    }

    uint32_t bx_min = APP_CAMERA_DISPLAY_WIDTH / 2U;
    uint32_t bx_max = 0U;
    for (uint32_t bx = 0U; bx < (APP_CAMERA_DISPLAY_WIDTH / 2U); bx++)
    {
      const uint32_t u0 = s_heat_u0[bx];
      /* Bilinear taps u0 and u0+1: active when either sits in the span. */
      if (((u0 + 1U) >= cell_min) && (u0 <= cell_max))
      {
        if (bx < bx_min)
        {
          bx_min = bx;
        }
        if (bx > bx_max)
        {
          bx_max = bx;
        }
      }
    }
    if (bx_min > bx_max)
    {
      continue;
    }

    const uint32_t px0 = bx_min * 2U;
    const uint32_t px_count = ((bx_max + 1U) * 2U) - px0;
    const uint32_t span_bytes = px_count * APP_CAMERA_DISPLAY_BYTES_PER_PIXEL;

    /* DCMIPP wrote this frame via DMA: refresh the touched span in the
     * cache, stage it in internal SRAM, blend there, then write back.
     * Per-pixel RMW straight against HyperRAM costs ~15x more. */
    AppCameraDisplay_InvalidateDCache((uint32_t)(dst0 + px0), span_bytes);
    AppCameraDisplay_InvalidateDCache((uint32_t)(dst1 + px0), span_bytes);
    (void)memcpy(&s_overlay_linebuf[0][px0], dst0 + px0, span_bytes);
    (void)memcpy(&s_overlay_linebuf[1][px0], dst1 + px0, span_bytes);
    if ((int32_t)(by * 2U) < s_overlay_dirty_y0)
    {
      s_overlay_dirty_y0 = (int32_t)(by * 2U);
    }
    if ((int32_t)((by * 2U) + 2U) > s_overlay_dirty_y1)
    {
      s_overlay_dirty_y1 = (int32_t)((by * 2U) + 2U);
    }

    {
      uint16_t *line0 = &s_overlay_linebuf[0][px0];
      uint16_t *line1 = &s_overlay_linebuf[1][px0];

      for (uint32_t bx = bx_min; bx <= bx_max; bx++)
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
        if (alpha != 0U)
        {
          const uint16_t color = palette[value & 0xFFU];
          const uint8_t a = (uint8_t)((alpha * qscale) >> 8);

          line0[0] = AppCameraDisplay_BlendRgb565(line0[0], color, a);
          line0[1] = AppCameraDisplay_BlendRgb565(line0[1], color, a);
          line1[0] = AppCameraDisplay_BlendRgb565(line1[0], color, a);
          line1[1] = AppCameraDisplay_BlendRgb565(line1[1], color, a);
        }

        line0 += 2;
        line1 += 2;
      }
    }

    (void)memcpy(dst0 + px0, &s_overlay_linebuf[0][px0], span_bytes);
    (void)memcpy(dst1 + px0, &s_overlay_linebuf[1][px0], span_bytes);
  }

  /* Fading dot trail of the primary source path. */
  if (overlay.trail_enabled != 0U)
  {
    for (uint32_t i = 0U; i < APP_CAMERA_DISPLAY_TRAIL_MAX; i++)
    {
      const AppCameraDisplayTrailDot_t *dot = &overlay.trail[i];

      if (dot->life == 0U)
      {
        continue;
      }

      {
        const int32_t dx = (int32_t)dot->x;
        const int32_t dy = (int32_t)dot->y;
        const int32_t dot_y0 = AppCameraDisplay_MaxI32(dy - 3, 0);
        const int32_t dot_y1 = AppCameraDisplay_MinI32(dy + 3, (int32_t)APP_CAMERA_DISPLAY_HEIGHT);
        const uint8_t a = (uint8_t)(30U + ((uint32_t)dot->life * 110U / 255U));
        const uint16_t trail_color = AppCameraDisplay_Rgb565(140U, 220U, 255U);

        if (dot_y1 <= dot_y0)
        {
          continue;
        }

        AppCameraDisplay_CleanInvalidateDCache(frame_addr +
                                                 ((uint32_t)dot_y0 * APP_CAMERA_DISPLAY_CAMERA_LINE_BYTES),
                                               (uint32_t)(dot_y1 - dot_y0) * APP_CAMERA_DISPLAY_CAMERA_LINE_BYTES);
        for (int32_t yy = dot_y0; yy < dot_y1; ++yy)
        {
          uint16_t *pixel_row = framebuffer + ((uint32_t)yy * APP_CAMERA_DISPLAY_WIDTH);
          const int32_t x0 = AppCameraDisplay_MaxI32(dx - 2, 0);
          const int32_t x1 = AppCameraDisplay_MinI32(dx + 2, (int32_t)APP_CAMERA_DISPLAY_WIDTH);

          for (int32_t xx = x0; xx < x1; ++xx)
          {
            pixel_row[xx] = AppCameraDisplay_BlendRgb565(pixel_row[xx], trail_color, a);
          }
        }

        if (dot_y0 < s_overlay_dirty_y0)
        {
          s_overlay_dirty_y0 = dot_y0;
        }
        if (dot_y1 > s_overlay_dirty_y1)
        {
          s_overlay_dirty_y1 = dot_y1;
        }
      }
    }
  }

  for (uint32_t i = 0U; i < overlay.marker_count; i++)
  {
    if (overlay.markers[i].strength >= APP_CAMERA_DISPLAY_MARKER_MIN_STRENGTH)
    {
      const int32_t band = 28;
      int32_t my0 = (int32_t)overlay.markers[i].y - band;
      int32_t my1 = (int32_t)overlay.markers[i].y + band;

      my0 = AppCameraDisplay_MaxI32(my0, 0);
      my1 = AppCameraDisplay_MinI32(my1, (int32_t)APP_CAMERA_DISPLAY_HEIGHT);
      if (my1 > my0)
      {
        AppCameraDisplay_CleanInvalidateDCache(frame_addr +
                                                 ((uint32_t)my0 * APP_CAMERA_DISPLAY_CAMERA_LINE_BYTES),
                                               (uint32_t)(my1 - my0) * APP_CAMERA_DISPLAY_CAMERA_LINE_BYTES);
        AppCameraDisplay_DrawMarker(framebuffer, &overlay.markers[i], (i == 0U) ? 1U : 0U);
        if (my0 < s_overlay_dirty_y0)
        {
          s_overlay_dirty_y0 = my0;
        }
        if (my1 > s_overlay_dirty_y1)
        {
          s_overlay_dirty_y1 = my1;
        }
      }
    }
  }

  /* Write the touched band back for the LTDC scan-out. */
  if (s_overlay_dirty_y1 > s_overlay_dirty_y0)
  {
    AppCameraDisplay_CleanDCache(frame_addr +
                                   ((uint32_t)s_overlay_dirty_y0 * APP_CAMERA_DISPLAY_CAMERA_LINE_BYTES),
                                 (uint32_t)(s_overlay_dirty_y1 - s_overlay_dirty_y0) *
                                   APP_CAMERA_DISPLAY_CAMERA_LINE_BYTES);
  }

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

/* The completed DCMIPP buffer is displayed directly (no compose copy); the
 * overlay draws into it before the vblank flip. Only the overlay-touched
 * region needs cache maintenance. */
static uint32_t AppCameraDisplay_PrepareDisplayFrame(uint32_t frame_addr)
{
  return frame_addr;
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

  /* Trail bookkeeping: age out old dots, push the primary position when it
   * has moved. Runs at UI tick rate (~60 Hz); life 255 with decay 3 gives
   * a dot roughly 1.4 s on screen. */
  if (s_acoustic_overlay.trail_enabled != 0U)
  {
    static uint16_t s_trail_last_x = 0xFFFFU;
    static uint16_t s_trail_last_y = 0xFFFFU;
    static uint8_t s_trail_slot = 0U;

    for (uint32_t i = 0U; i < APP_CAMERA_DISPLAY_TRAIL_MAX; i++)
    {
      if (s_acoustic_overlay.trail[i].life > 3U)
      {
        s_acoustic_overlay.trail[i].life -= 3U;
      }
      else
      {
        s_acoustic_overlay.trail[i].life = 0U;
      }
    }

    if ((s_acoustic_overlay.enabled != 0U) && (s_acoustic_overlay.marker_count != 0U))
    {
      const uint16_t px = s_acoustic_overlay.markers[0].x;
      const uint16_t py = s_acoustic_overlay.markers[0].y;
      const int32_t moved_x = (int32_t)px - (int32_t)s_trail_last_x;
      const int32_t moved_y = (int32_t)py - (int32_t)s_trail_last_y;

      if (((moved_x * moved_x) + (moved_y * moved_y)) >= 64)
      {
        s_acoustic_overlay.trail[s_trail_slot].x = px;
        s_acoustic_overlay.trail[s_trail_slot].y = py;
        s_acoustic_overlay.trail[s_trail_slot].life = 255U;
        s_trail_slot = (uint8_t)((s_trail_slot + 1U) % APP_CAMERA_DISPLAY_TRAIL_MAX);
        s_trail_last_x = px;
        s_trail_last_y = py;
      }
    }
  }
  else
  {
    memset(s_acoustic_overlay.trail, 0, sizeof(s_acoustic_overlay.trail));
  }

  if (primask == 0U)
  {
    __enable_irq();
  }

  g_app_camera_overlay_update_count++;
}

void AppCameraDisplay_SetTrailEnabled(uint8_t enabled)
{
  s_acoustic_overlay.trail_enabled = (enabled != 0U) ? 1U : 0U;
}

uint8_t AppCameraDisplay_GetTrailEnabled(void)
{
  return s_acoustic_overlay.trail_enabled;
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

/* Called from the DCMIPP frame ISR: only records the newest completed frame
 * and wakes the worker. Overwrites any not-yet-consumed frame (latest wins). */
void AppCameraDisplay_RequestSwap(uint32_t frame_addr)
{
  if (((g_app_camera_display_flags & APP_CAMERA_DISPLAY_FLAG_READY) == 0U) || (frame_addr == 0U))
  {
    return;
  }

  s_swap_pending_addr = frame_addr;
  if (s_swap_worker_ready != 0U)
  {
    (void)tx_event_flags_set(&s_swap_events, APP_CAMERA_DISPLAY_SWAP_EVENT, TX_OR);
  }
}

/* Overlay + LTDC flip for one camera frame (worker context). */
static void AppCameraDisplay_ProcessSwap(uint32_t frame_addr)
{
  uint32_t display_addr;

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
    /* Cache maintenance happens inside the draw, per touched scanline band. */
    AppCameraDisplay_DrawAcousticOverlay(display_addr);
    LTDC_Layer1->CFBAR = display_addr;
    AppCameraDisplay_ReloadLayer(LTDC_Layer1, LTDC_LxRCR_VBR);

    /* Debug screenshot freeze: this frame is fully composed and flipped;
     * parking DMA now (only when the overlay is live) preserves exactly
     * what the screen shows. */
    AppCamera_FreezeIfRequested(s_acoustic_overlay.enabled);
  }
  g_app_camera_ltdc_swap_count++;
  AppCameraDisplay_SnapshotLtdc();
}

UINT AppCameraDisplay_WorkerInit(void)
{
  UINT status = tx_event_flags_create(&s_swap_events, "cam_swap");

  if (status == TX_SUCCESS)
  {
    s_swap_worker_ready = 1U;
  }
  return status;
}

void AppCameraDisplay_WorkerThreadEntry(ULONG thread_input)
{
  ULONG events;

  (void)thread_input;

  while (1)
  {
    if (tx_event_flags_get(&s_swap_events,
                           APP_CAMERA_DISPLAY_SWAP_EVENT,
                           TX_OR_CLEAR,
                           &events,
                           TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      tx_thread_sleep(1U);
      continue;
    }

    {
      /* Consume the newest pending frame; RequestSwap may overwrite this
       * while we draw, which simply queues the next wakeup. */
      const uint32_t frame_addr = s_swap_pending_addr;

      s_swap_pending_addr = 0U;
      if (frame_addr != 0U)
      {
        AppCameraDisplay_ProcessSwap(frame_addr);
      }
    }
  }
}

uint8_t AppCameraDisplay_CopyDisplayedFrame(uint16_t *dst)
{
  const uint32_t frame_addr = g_app_camera_display_addr;
  const uint32_t frame_bytes =
    APP_CAMERA_DISPLAY_WIDTH * APP_CAMERA_DISPLAY_HEIGHT * APP_CAMERA_DISPLAY_BYTES_PER_PIXEL;

  if ((dst == 0) ||
      ((g_app_camera_display_flags & APP_CAMERA_DISPLAY_FLAG_VISIBLE) == 0U) ||
      (frame_addr == 0U))
  {
    return 0U;
  }

  /* The frame was produced by DCMIPP DMA plus the overlay worker; drop any
   * stale cache lines before the CPU reads it. */
  AppCameraDisplay_InvalidateDCache(frame_addr, frame_bytes);
  memcpy(dst, (const void *)frame_addr, frame_bytes);
  return 1U;
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
