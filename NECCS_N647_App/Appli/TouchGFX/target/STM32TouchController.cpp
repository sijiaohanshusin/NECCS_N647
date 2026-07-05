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
#include "stm32n6xx_hal.h"

extern "C"
{
#include "TOUCH/app_touch.h"
}

#define APP_TOUCH_CONTROLLER_POLL_MS 20U

static bool appTouchTimeReached(uint32_t now, uint32_t target)
{
    return static_cast<int32_t>(now - target) >= 0;
}

void STM32TouchController::init()
{
    (void)AppTouch_Init();
}

bool STM32TouchController::sampleTouch(int32_t& x, int32_t& y)
{
    static uint32_t nextPollMs = 0U;
    static bool cachedDown = false;
    static int32_t cachedX = 0;
    static int32_t cachedY = 0;
    uint16_t touchX = 0U;
    uint16_t touchY = 0U;
    const uint32_t now = HAL_GetTick();

    if ((nextPollMs != 0U) && !appTouchTimeReached(now, nextPollMs))
    {
        if (!cachedDown)
        {
            return false;
        }

        x = cachedX;
        y = cachedY;
        return true;
    }

    nextPollMs = now + APP_TOUCH_CONTROLLER_POLL_MS;

    if (AppTouch_Sample(&touchX, &touchY) == 0U)
    {
        cachedDown = false;
        return false;
    }

    cachedX = static_cast<int32_t>(touchX);
    cachedY = static_cast<int32_t>(touchY);
    cachedDown = true;
    x = cachedX;
    y = cachedY;
    return true;
}

/* USER CODE END STM32TouchController */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
