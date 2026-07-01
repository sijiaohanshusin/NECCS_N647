/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_bringup_thread.h"
#include "app_debug_config.h"
#include "app_i2c2_bus.h"
#include "app_media.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_BRINGUP_THREAD_STACK_SIZE  4096U
#define APP_BRINGUP_THREAD_PRIORITY    18U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TX_THREAD app_bringup_thread;
static ULONG app_bringup_thread_stack[APP_BRINGUP_THREAD_STACK_SIZE / sizeof(ULONG)];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  /* USER CODE BEGIN App_ThreadX_MEM_POOL */
  (void)memory_ptr;

  /* USER CODE END App_ThreadX_MEM_POOL */
  /* USER CODE BEGIN App_ThreadX_Init */
  ret = AppI2C2_BusInit();
  if (ret == TX_SUCCESS)
  {
    ret = tx_thread_create(&app_bringup_thread,
                           "app_bringup",
                           App_BringUpThreadEntry,
                           0U,
                           app_bringup_thread_stack,
                           APP_BRINGUP_THREAD_STACK_SIZE,
                           APP_BRINGUP_THREAD_PRIORITY,
                           APP_BRINGUP_THREAD_PRIORITY,
                           TX_NO_TIME_SLICE,
                           TX_AUTO_START);
  }
  if (ret == TX_SUCCESS)
  {
#if APP_TEMP_BQ_DEBUG_MODE
    /* Field BQ debug mode keeps SD/FileX out of the boot path. */
#else
    ret = AppMedia_Init(memory_ptr);
#endif
  }
  /* USER CODE END App_ThreadX_Init */

  return ret;
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN Before_Kernel_Start */

  /* USER CODE END Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN Kernel_Start_Error */

  /* USER CODE END Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
