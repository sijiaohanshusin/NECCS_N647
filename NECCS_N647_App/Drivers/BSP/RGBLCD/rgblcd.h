/**
  ******************************************************************************
  * @file    rgblcd.h
  * @brief   RGB LCD panel bring-up (panel detect, LTDC timing/clock, backlight).
  ******************************************************************************
  */

#ifndef __RGBLCD_H
#define __RGBLCD_H

#include "main.h"

/* RGB LCD部分引脚定义 */
/* N647 baseboard LCD backlight is PA3; LCD_BL_PWM/PE9 belongs to another board label set. */
#define RGBLCD_BL_GPIO_PORT GPIOA
#define RGBLCD_BL_GPIO_PIN  GPIO_PIN_3

#if defined(LCD_NRST_GPIO_Port) && defined(LCD_NRST_Pin)
#define RGBLCD_RST_GPIO_PORT LCD_NRST_GPIO_Port
#define RGBLCD_RST_GPIO_PIN  LCD_NRST_Pin
#endif

#define RGBLCD_B7_GPIO_PORT GPIOA
#define RGBLCD_B7_GPIO_PIN  GPIO_PIN_2
#define RGBLCD_B7_GPIO_AF   GPIO_AF14_LCD

#define RGBLCD_G7_GPIO_PORT GPIOB
#define RGBLCD_G7_GPIO_PIN  GPIO_PIN_10
#define RGBLCD_G7_GPIO_AF   GPIO_AF14_LCD

#define RGBLCD_R7_GPIO_PORT GPIOG
#define RGBLCD_R7_GPIO_PIN  GPIO_PIN_9
#define RGBLCD_R7_GPIO_AF   GPIO_AF14_LCD

/* RGB LCD引脚操作 */
#define RGBLCD_BL(x)        do { (x) ?                                                                      \
                                HAL_GPIO_WritePin(RGBLCD_BL_GPIO_PORT, RGBLCD_BL_GPIO_PIN, GPIO_PIN_SET):   \
                                HAL_GPIO_WritePin(RGBLCD_BL_GPIO_PORT, RGBLCD_BL_GPIO_PIN, GPIO_PIN_RESET); \
                            } while (0)

#if defined(RGBLCD_RST_GPIO_PORT) && defined(RGBLCD_RST_GPIO_PIN)
#define RGBLCD_RESET(x)     do { (x) ?                                                                         \
                                HAL_GPIO_WritePin(RGBLCD_RST_GPIO_PORT, RGBLCD_RST_GPIO_PIN, GPIO_PIN_SET):    \
                                HAL_GPIO_WritePin(RGBLCD_RST_GPIO_PORT, RGBLCD_RST_GPIO_PIN, GPIO_PIN_RESET);  \
                            } while (0)
#else
#define RGBLCD_RESET(x)     do { (void)(x); } while (0)
#endif

/* RGB LCD重要参数集 */
typedef struct
{
    uint16_t id;                    /* LCD ID */
    uint32_t pwidth;                /* LCD面板的宽度 */
    uint32_t pheight;               /* LCD面板的高度 */
    uint16_t hsw;                   /* 水平同步宽度 */
    uint16_t vsw;                   /* 垂直同步宽度 */
    uint16_t hbp;                   /* 水平后廊 */
    uint16_t vbp;                   /* 垂直后廊 */
    uint16_t hfp;                   /* 水平前廊 */
    uint16_t vfp;                   /* 垂直前廊 */
    uint8_t dir;                    /* 横竖屏标志位，0：竖屏；1：横屏 */
    uint16_t width;                 /* LCD宽度 */
    uint16_t height;                /* LCD高度 */
} _rgblcd_dev;

/* RGB LCD参数导出 */
extern _rgblcd_dev rgblcddev;
extern uint16_t g_ltdc_lcd_framebuf[1280 * 800] __attribute__((section(".EXTRAM")));
extern volatile uint16_t g_rgblcd_raw_panel_id;
extern volatile uint16_t g_rgblcd_effective_panel_id;
extern volatile uint32_t g_rgblcd_init_stage;
extern volatile uint32_t g_rgblcd_ltdc_requested_clock;
extern volatile uint32_t g_rgblcd_ltdc_clock_divider;
extern volatile uint32_t g_rgblcd_ltdc_actual_clock;
extern volatile uint32_t g_rgblcd_ltdc_clk_status;

/* 函数声明 */
void rgblcd_init(void);     /* 初始化RGB LCD面板/LTDC时序/背光 */

#endif
