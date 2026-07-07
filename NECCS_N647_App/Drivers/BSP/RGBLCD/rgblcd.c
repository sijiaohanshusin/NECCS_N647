/**
  ******************************************************************************
  * @file    rgblcd.c
  * @brief   RGB LCD panel bring-up: panel detect, LTDC timing/clock, backlight.
  *          Derived from the ALIENTEK N647 reference driver; the legacy
  *          drawing/text API was removed because all rendering goes through
  *          TouchGFX and the camera display path.
  ******************************************************************************
  */

#include "rgblcd.h"
#include <stdint.h>

extern LTDC_HandleTypeDef hltdc;
extern DMA2D_HandleTypeDef hdma2d;

/* RGB LCD panel parameters */
_rgblcd_dev rgblcddev;

#define RGBLCD_PANEL_ATK_MD0700R_1024600  0x7016U

/* Current NECCS_N647 bring-up hardware uses the 1024x600 RGBLCD even if ID straps read otherwise. */
#ifndef RGBLCD_FORCE_PANEL_ID
#define RGBLCD_FORCE_PANEL_ID             RGBLCD_PANEL_ATK_MD0700R_1024600
#endif

volatile uint16_t g_rgblcd_raw_panel_id = 0;
volatile uint16_t g_rgblcd_effective_panel_id = 0;
volatile uint32_t g_rgblcd_init_stage = 0;
volatile uint32_t g_rgblcd_ltdc_requested_clock = 0;
volatile uint32_t g_rgblcd_ltdc_clock_divider = 0;
volatile uint32_t g_rgblcd_ltdc_actual_clock = 0;
volatile uint32_t g_rgblcd_ltdc_clk_status = 0;

/* Initial LTDC layer-0 framebuffer; TouchGFX takes over the layer afterwards. */
uint16_t g_ltdc_lcd_framebuf[1280 * 800] __attribute__((section(".EXTRAM")));

static uint16_t rgblcd_panelid_read(void);
static void rgblcd_use_default_panel(void);
static uint8_t rgblcd_ltdc_clk_set(uint32_t clock);
static void rgblcd_display_dir(uint8_t dir);
static void rgblcd_clear(uint16_t color);
static void rgblcd_cache_clean_invalidate_region(void *addr, uint32_t bytes);
static void rgblcd_cache_invalidate_region(void *addr, uint32_t bytes);

static void rgblcd_cache_clean_invalidate_region(void *addr, uint32_t bytes)
{
    if ((addr == NULL) || (bytes == 0U) || ((SCB->CCR & SCB_CCR_DC_Msk) == 0U))
    {
        return;
    }

    uintptr_t start = ((uintptr_t)addr) & ~(uintptr_t)31U;
    uintptr_t end = (((uintptr_t)addr) + bytes + 31U) & ~(uintptr_t)31U;

    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)start, (int32_t)(end - start));
    __DSB();
    __ISB();
}

static void rgblcd_cache_invalidate_region(void *addr, uint32_t bytes)
{
    if ((addr == NULL) || (bytes == 0U) || ((SCB->CCR & SCB_CCR_DC_Msk) == 0U))
    {
        return;
    }

    uintptr_t start = ((uintptr_t)addr) & ~(uintptr_t)31U;
    uintptr_t end = (((uintptr_t)addr) + bytes + 31U) & ~(uintptr_t)31U;

    SCB_InvalidateDCache_by_Addr((void *)start, (int32_t)(end - start));
    __DSB();
    __ISB();
}

void rgblcd_init(void)
{
    LTDC_LayerCfgTypeDef ltdc_layer_cfg_struct = {0};

    g_rgblcd_init_stage = 1;
    RGBLCD_BL(0);
    RGBLCD_RESET(0);
    HAL_Delay(10);
    RGBLCD_RESET(1);
    HAL_Delay(120);

    g_rgblcd_raw_panel_id = rgblcd_panelid_read();
#if (RGBLCD_FORCE_PANEL_ID != 0U)
    rgblcddev.id = RGBLCD_FORCE_PANEL_ID;
#else
    rgblcddev.id = g_rgblcd_raw_panel_id;
#endif
    if (rgblcddev.id == 0x4342)         /* ATK-MD0430R-480272 */
    {
        rgblcddev.pwidth = 480;
        rgblcddev.pheight = 272;
        rgblcddev.hsw = 1;
        rgblcddev.vsw = 1;
        rgblcddev.hbp = 40;
        rgblcddev.vbp = 8;
        rgblcddev.hfp = 5;
        rgblcddev.vfp = 8;
    }
    else if (rgblcddev.id == 0x7084)    /* ATK-MD0700R-800480 */
    {
        rgblcddev.pwidth = 800;
        rgblcddev.pheight = 480;
        rgblcddev.hsw = 1;
        rgblcddev.vsw = 1;
        rgblcddev.hbp = 46;
        rgblcddev.vbp = 23;
        rgblcddev.hfp = 210;
        rgblcddev.vfp = 22;
    }
    else if (rgblcddev.id == 0x7016)    /* ATK-MD0700R-1024600 */
    {
        rgblcddev.pwidth = 1024;
        rgblcddev.pheight = 600;
        rgblcddev.hsw = 20;
        rgblcddev.vsw = 3;
        rgblcddev.hbp = 140;
        rgblcddev.vbp = 20;
        rgblcddev.hfp = 160;
        rgblcddev.vfp = 12;
    }
    else if (rgblcddev.id == 0x4384)    /* ATK-MD0430R-800480 */
    {
        rgblcddev.pwidth = 800;
        rgblcddev.pheight = 480;
        rgblcddev.hsw = 88;
        rgblcddev.vsw = 40;
        rgblcddev.hbp = 48;
        rgblcddev.vbp = 32;
        rgblcddev.hfp = 13;
        rgblcddev.vfp = 3;
    }
    else if (rgblcddev.id == 0x1018)    /* ATK-MD1018R-1280800 */
    {
        rgblcddev.pwidth = 1280;
        rgblcddev.pheight = 800;
        rgblcddev.hsw = 140;
        rgblcddev.vsw = 10;
        rgblcddev.hbp = 10;
        rgblcddev.vbp = 10;
        rgblcddev.hfp = 10;
        rgblcddev.vfp = 3;
    }

    if ((rgblcddev.pwidth == 0U) || (rgblcddev.pheight == 0U) ||
        (rgblcddev.hsw == 0U) || (rgblcddev.vsw == 0U))
    {
        rgblcd_use_default_panel();
    }

    g_rgblcd_effective_panel_id = rgblcddev.id;
    g_rgblcd_init_stage = 2;

    hltdc.Init.PCPolarity = (rgblcddev.id == 0x1018) ? LTDC_PCPOLARITY_IIPC : LTDC_PCPOLARITY_IPC;
    hltdc.Init.HorizontalSync = rgblcddev.hsw - 1;
    hltdc.Init.VerticalSync = rgblcddev.vsw - 1;
    hltdc.Init.AccumulatedHBP = rgblcddev.hsw + rgblcddev.hbp - 1;
    hltdc.Init.AccumulatedVBP = rgblcddev.vsw + rgblcddev.vbp - 1;
    hltdc.Init.AccumulatedActiveW = rgblcddev.hsw + rgblcddev.hbp + rgblcddev.pwidth - 1;
    hltdc.Init.AccumulatedActiveH = rgblcddev.vsw + rgblcddev.vbp + rgblcddev.pheight - 1;
    hltdc.Init.TotalWidth = rgblcddev.hsw + rgblcddev.hbp + rgblcddev.pwidth + rgblcddev.hfp - 1;
    hltdc.Init.TotalHeigh = rgblcddev.vsw + rgblcddev.vbp + rgblcddev.pheight + rgblcddev.vfp - 1;
    HAL_LTDC_DeInit(&hltdc);
    HAL_LTDC_Init(&hltdc);

    if (rgblcddev.id == 0x4342)
    {
        rgblcd_ltdc_clk_set(9000000);   /* LTDC_CLK = 9MHz */
    }
    else if ((rgblcddev.id == 0x7084) || (rgblcddev.id == 0x4384))
    {
        rgblcd_ltdc_clk_set(33333333);  /* LTDC_CLK = 33MHz */
    }
    else if (rgblcddev.id == 0x7016)
    {
        rgblcd_ltdc_clk_set(51200000);  /* LTDC_CLK ~= 51.2MHz */
    }
    else if (rgblcddev.id == 0x1018)
    {
        rgblcd_ltdc_clk_set(45000000);  /* LTDC_CLK = 45MHz */
    }

    ltdc_layer_cfg_struct.WindowX0 = 0;
    ltdc_layer_cfg_struct.WindowX1 = rgblcddev.pwidth;
    ltdc_layer_cfg_struct.WindowY0 = 0;
    ltdc_layer_cfg_struct.WindowY1 = rgblcddev.pheight;
    ltdc_layer_cfg_struct.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    ltdc_layer_cfg_struct.Alpha = 255;
    ltdc_layer_cfg_struct.Alpha0 = 0;
    ltdc_layer_cfg_struct.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
    ltdc_layer_cfg_struct.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
    ltdc_layer_cfg_struct.FBStartAdress = 0;
    ltdc_layer_cfg_struct.ImageWidth = rgblcddev.pwidth;
    ltdc_layer_cfg_struct.ImageHeight = rgblcddev.pheight;
    ltdc_layer_cfg_struct.Backcolor.Blue = 0;
    ltdc_layer_cfg_struct.Backcolor.Green = 0;
    ltdc_layer_cfg_struct.Backcolor.Red = 0;
    HAL_LTDC_ConfigLayer(&hltdc, &ltdc_layer_cfg_struct, 0);
    HAL_LTDC_SetAddress(&hltdc, (uint32_t)g_ltdc_lcd_framebuf, 0);

    rgblcd_display_dir(0);
    g_rgblcd_init_stage = 3;
    rgblcd_clear(0xFFFF);
    g_rgblcd_init_stage = 4;
    RGBLCD_BL(1);
    g_rgblcd_init_stage = 5;
}

static void rgblcd_display_dir(uint8_t dir)
{
    rgblcddev.dir = dir;
    if (rgblcddev.dir != 0)
    {
        rgblcddev.width = rgblcddev.pwidth;
        rgblcddev.height = rgblcddev.pheight;
    }
    else
    {
        rgblcddev.width = rgblcddev.pheight;
        rgblcddev.height = rgblcddev.pwidth;
    }
}

/* DMA2D register-to-memory fill of the whole panel with one RGB565 color. */
static void rgblcd_clear(uint16_t color)
{
#define CONVERTRGB5652ARGB8888(Color)                                               \
            ((((((((Color) >> (11U)) & 0x1FU) * 527U) + 23U) >> (6U)) << (16U)) |   \
            (((((((Color) >> (5U)) & 0x3FU) * 259U) + 33U) >> (6U)) << (8U)) |      \
            (((((Color) & 0x1FU) * 527U) + 23U) >> (6U)) | (0xFF000000U))

    if ((rgblcddev.pwidth == 0U) || (rgblcddev.pheight == 0U))
    {
        return;
    }

    uint32_t cache_bytes = rgblcddev.pwidth * rgblcddev.pheight * (uint32_t)sizeof(uint16_t);

    rgblcd_cache_clean_invalidate_region(g_ltdc_lcd_framebuf, cache_bytes);

    hdma2d.Init.Mode = DMA2D_R2M;
    hdma2d.Init.OutputOffset = 0;
    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_Start(&hdma2d, CONVERTRGB5652ARGB8888(color), (uint32_t)g_ltdc_lcd_framebuf,
                    rgblcddev.pwidth, rgblcddev.pheight);
    HAL_DMA2D_PollForTransfer(&hdma2d, 50);
    rgblcd_cache_invalidate_region(g_ltdc_lcd_framebuf, cache_bytes);
}

static uint16_t rgblcd_panelid_read(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    uint16_t pids[] = {
        0x4342, /* ATK-MD0430R-480272 */
        0x7084, /* ATK-MD0700R-800480 */
        0x7016, /* ATK-MD0700R-1024600 */
        0x7018, /* ATK-MD0700R-1280800 */
        0x4384, /* ATK-MD0430R-800480 */
        0x1018, /* ATK-MD1018R-1280800 */
        0xFFFF, /* Unknown */
    };
    uint8_t id;

    gpio_init_struct.Pin = RGBLCD_B7_GPIO_PIN;              /* LTDC_B7 */
    gpio_init_struct.Mode = GPIO_MODE_INPUT;
    gpio_init_struct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(RGBLCD_B7_GPIO_PORT, &gpio_init_struct);

    gpio_init_struct.Pin = RGBLCD_G7_GPIO_PIN;              /* LTDC_G7 */
    HAL_GPIO_Init(RGBLCD_G7_GPIO_PORT, &gpio_init_struct);

    gpio_init_struct.Pin = RGBLCD_R7_GPIO_PIN;              /* LTDC_R7 */
    HAL_GPIO_Init(RGBLCD_R7_GPIO_PORT, &gpio_init_struct);

    HAL_Delay(100);

    id = ((HAL_GPIO_ReadPin(RGBLCD_R7_GPIO_PORT, RGBLCD_R7_GPIO_PIN) == GPIO_PIN_RESET) ? 0 : 1);
    id |= ((HAL_GPIO_ReadPin(RGBLCD_G7_GPIO_PORT, RGBLCD_G7_GPIO_PIN) == GPIO_PIN_RESET) ? 0 : 1) << 1;
    id |= ((HAL_GPIO_ReadPin(RGBLCD_B7_GPIO_PORT, RGBLCD_B7_GPIO_PIN) == GPIO_PIN_RESET) ? 0 : 1) << 2;

    if (id >= (sizeof(pids) / sizeof(pids[0]))) {
        id = sizeof(pids) / sizeof(pids[0]) - 1;
    }

    gpio_init_struct.Pin = RGBLCD_B7_GPIO_PIN;              /* LTDC_B7 */
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_struct.Alternate = RGBLCD_B7_GPIO_AF;
    HAL_GPIO_Init(RGBLCD_B7_GPIO_PORT, &gpio_init_struct);

    gpio_init_struct.Pin = RGBLCD_G7_GPIO_PIN;              /* LTDC_G7 */
    gpio_init_struct.Alternate = RGBLCD_G7_GPIO_AF;
    HAL_GPIO_Init(RGBLCD_G7_GPIO_PORT, &gpio_init_struct);

    gpio_init_struct.Pin = RGBLCD_R7_GPIO_PIN;              /* LTDC_R7 */
    gpio_init_struct.Alternate = RGBLCD_R7_GPIO_AF;
    HAL_GPIO_Init(RGBLCD_R7_GPIO_PORT, &gpio_init_struct);

    return pids[id];
}

static void rgblcd_use_default_panel(void)
{
    rgblcddev.id = RGBLCD_PANEL_ATK_MD0700R_1024600;      /* ATK-MD0700R-1024600 */
    rgblcddev.pwidth = 1024;
    rgblcddev.pheight = 600;
    rgblcddev.hsw = 20;
    rgblcddev.vsw = 3;
    rgblcddev.hbp = 140;
    rgblcddev.vbp = 20;
    rgblcddev.hfp = 160;
    rgblcddev.vfp = 12;
}

/**
 * @brief   Configure the LTDC pixel clock from PLL1 through IC16.
 * @retval  0: success, 1: failure (diagnostics in g_rgblcd_ltdc_* globals)
 */
static uint8_t rgblcd_ltdc_clk_set(uint32_t clock)
{
    RCC_PeriphCLKInitTypeDef rcc_periph_clk_init_struct = {0};
    uint32_t pll1_clock = HAL_RCCEx_GetPLL1CLKFreq();
    uint32_t divider;

    g_rgblcd_ltdc_requested_clock = clock;
    g_rgblcd_ltdc_clock_divider = 0;
    g_rgblcd_ltdc_actual_clock = 0;
    g_rgblcd_ltdc_clk_status = 1;

    if ((clock == 0U) || (pll1_clock == 0U))
    {
        return 1;
    }

    divider = (pll1_clock + (clock / 2U)) / clock;
    if (divider == 0U)
    {
        divider = 1U;
    }

    rcc_periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    rcc_periph_clk_init_struct.LtdcClockSelection = RCC_LTDCCLKSOURCE_IC16;
    rcc_periph_clk_init_struct.ICSelection[RCC_IC16].ClockSelection = RCC_ICCLKSOURCE_PLL1;
    rcc_periph_clk_init_struct.ICSelection[RCC_IC16].ClockDivider = divider;
    if (HAL_RCCEx_PeriphCLKConfig(&rcc_periph_clk_init_struct) != HAL_OK)
    {
        return 1;
    }

    g_rgblcd_ltdc_clock_divider = divider;
    g_rgblcd_ltdc_actual_clock = pll1_clock / divider;
    g_rgblcd_ltdc_clk_status = 0;

    return 0;
}
