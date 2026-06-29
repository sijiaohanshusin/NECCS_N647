/**
  ******************************************************************************
  * File Name          : app_touchgfx.c
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

/* Includes ------------------------------------------------------------------*/
#include "tx_api.h"
#include "app_touchgfx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_boot_diag.h"

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
#define TOUCHGFX_STACK_SIZE          (4080)

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static TX_THREAD TouchGFXThread;

/* USER CODE BEGIN PV */
static UINT touchgfx_preos_initialized = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void touchgfx_init(void);
void touchgfx_components_init(void);
void touchgfx_taskEntry(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
 * PreOS Initialization function
 */
void MX_TouchGFX_PreOSInit(void)
{
    if (touchgfx_preos_initialized != 0U)
    {
        return;
    }

    App_BootDiag_SetStage(APP_BOOT_STAGE_TOUCHGFX_PREOS_START);

    // Calling forward to touchgfx_init in C++ domain
    touchgfx_components_init();
    touchgfx_init();

    touchgfx_preos_initialized = 1U;
    g_app_boot_diag.touchgfx_preos_count++;
    App_BootDiag_SetStage(APP_BOOT_STAGE_TOUCHGFX_PREOS_DONE);
}

/**
 * Create TouchGFX Thread
 */
UINT MX_TouchGFX_Init(VOID* memory_ptr)
{
    UINT ret = TX_SUCCESS;
    UINT status = TX_SUCCESS;
    CHAR* pointer = 0;

    /* Allocate the stack for TouchGFX Thread.  */
    status = tx_byte_allocate((TX_BYTE_POOL*)memory_ptr, (VOID**) &pointer,
                              TOUCHGFX_STACK_SIZE, TX_NO_WAIT);
    g_app_boot_diag.touchgfx_alloc_status = status;
    if (status != TX_SUCCESS)
    {
        ret = TX_POOL_ERROR;
    }

    /* Create TouchGFX Thread */
    else
    {
        status = tx_thread_create(&TouchGFXThread, (CHAR*)"TouchGFX", TouchGFX_Task, 0,
                                  pointer, TOUCHGFX_STACK_SIZE,
                                  5, 5,
                                  TX_NO_TIME_SLICE, TX_AUTO_START);
        g_app_boot_diag.touchgfx_thread_status = status;
        if (status != TX_SUCCESS)
        {
            ret = TX_THREAD_ERROR;
        }
    }

    return ret;
}

/**
 * TouchGFX application entry function
 */
void MX_TouchGFX_Process(void)
{
    MX_TouchGFX_PreOSInit();
    App_BootDiag_SetStage(APP_BOOT_STAGE_TOUCHGFX_PROCESS);

    // Calling forward to touchgfx_taskEntry in C++ domain
    touchgfx_taskEntry();
}

/**
 * TouchGFX application thread
 */
void TouchGFX_Task(unsigned long thread_input)
{
    (void)thread_input;
    App_BootDiag_SetStage(APP_BOOT_STAGE_TOUCHGFX_THREAD_ENTER);
    MX_TouchGFX_PreOSInit();

    // Calling forward to touchgfx_taskEntry in C++ domain
    touchgfx_taskEntry();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
