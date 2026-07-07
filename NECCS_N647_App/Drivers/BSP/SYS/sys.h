/**
  ******************************************************************************
  * @file    sys.h
  * @brief   Debug system clock and SAI audio clock configuration.
  ******************************************************************************
  */

#ifndef __SYS_H
#define __SYS_H

#include "main.h"

uint8_t sys_audio_clock_config(void);   /* SAI1音频时钟树配置 */
void sys_clock_config_debug(void);      /* Debug/RAM启动路径系统时钟 */

#endif
