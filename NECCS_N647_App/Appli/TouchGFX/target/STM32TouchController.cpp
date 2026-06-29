/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : STM32TouchController.cpp
  ******************************************************************************
  * This file was created by TouchGFX Generator 4.26.1. This file is only
  * generated once! Delete this file from your project and re-generate code
  * using STM32CubeMX or change this file manually to update it.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* USER CODE BEGIN STM32TouchController */

#include <STM32TouchController.hpp>

extern "C"
{
#include "TOUCH/app_touch.h"
}

void STM32TouchController::init()
{
    (void)AppTouch_Init();
}

bool STM32TouchController::sampleTouch(int32_t& x, int32_t& y)
{
    uint16_t touchX = 0U;
    uint16_t touchY = 0U;

    if (AppTouch_Sample(&touchX, &touchY) == 0U)
    {
        return false;
    }

    x = static_cast<int32_t>(touchX);
    y = static_cast<int32_t>(touchY);
    return true;
}

/* USER CODE END STM32TouchController */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
