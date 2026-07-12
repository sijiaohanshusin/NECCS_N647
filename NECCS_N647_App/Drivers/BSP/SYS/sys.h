/**
  ******************************************************************************
  * @file    sys.h
  * @brief   Debug system clock and SAI audio clock configuration.
  ******************************************************************************
  */

#ifndef __SYS_H
#define __SYS_H

#include "main.h"

/* SAI1 kernel-clock targets derived from the shared 245.76 MHz PLL2 (HSI
 * source, no external crystal). Both divide exactly:
 *   Wide32@48k : 245.76 / 20 = 12.288 MHz (= 48 k x 16 slot x 16 bit)
 *   Core16@192k: 245.76 / 10 = 24.576 MHz (= 192 k x 8 slot x 16 bit)  */
#define SYS_AUDIO_SAI1_FREQ_WIDE32_HZ   12288000UL
#define SYS_AUDIO_SAI1_FREQ_CORE16_HZ   24576000UL

uint8_t sys_audio_clock_config(uint32_t sai1_kernel_hz);  /* SAI1音频时钟树配置 */
void sys_clock_config_debug(void);      /* Debug/RAM启动路径系统时钟 */

#endif
