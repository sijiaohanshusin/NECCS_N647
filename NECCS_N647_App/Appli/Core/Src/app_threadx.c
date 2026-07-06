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
#include "app_acoustic_service.h"
#include "app_i2c2_bus.h"
#include "app_media.h"
#include "app_pcmd_capture.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_BRINGUP_THREAD_STACK_SIZE  4096U
#define APP_BRINGUP_THREAD_PRIORITY    11U
#define APP_PCMD_THREAD_STACK_SIZE     8192U
#define APP_PCMD_THREAD_PRIORITY       12U
#define APP_ACOUSTIC_THREAD_STACK_SIZE 12288U
#define APP_ACOUSTIC_THREAD_PRIORITY   13U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TX_THREAD app_bringup_thread;
static ULONG app_bringup_thread_stack[APP_BRINGUP_THREAD_STACK_SIZE / sizeof(ULONG)];
static TX_THREAD app_pcmd_thread;
static ULONG app_pcmd_thread_stack[APP_PCMD_THREAD_STACK_SIZE / sizeof(ULONG)];
static TX_THREAD app_acoustic_thread;
static ULONG app_acoustic_thread_stack[APP_ACOUSTIC_THREAD_STACK_SIZE / sizeof(ULONG)];

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
    App_BringUpStatus_Ready(APP_BRINGUP_MODULE_I2C, (int32_t)ret);
  }
  else
  {
    App_BringUpStatus_Fail(APP_BRINGUP_MODULE_I2C, (int32_t)ret);
  }
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
    if (ret != TX_SUCCESS)
    {
      App_BringUpStatus_Fail(APP_BRINGUP_MODULE_CAMERA, (int32_t)ret);
    }
  }
  if (ret == TX_SUCCESS)
  {
    if (App_BringUpControl_IsEnabled(APP_BRINGUP_CONTROL_PCMD_RAW) != 0U)
    {
      App_BringUpStatus_Start(APP_BRINGUP_MODULE_PCMD_RAW, 0);
      ret = tx_thread_create(&app_pcmd_thread,
                             "app_pcmd_capture",
                             AppPcmdCapture_ThreadEntry,
                             0U,
                             app_pcmd_thread_stack,
                             APP_PCMD_THREAD_STACK_SIZE,
                             APP_PCMD_THREAD_PRIORITY,
                             APP_PCMD_THREAD_PRIORITY,
                             TX_NO_TIME_SLICE,
                             TX_AUTO_START);
      if (ret != TX_SUCCESS)
      {
        App_BringUpStatus_Fail(APP_BRINGUP_MODULE_PCMD_RAW, (int32_t)ret);
      }
    }
    else
    {
      App_BringUpStatus_Skip(APP_BRINGUP_MODULE_PCMD_RAW, 0);
      App_BringUpStatus_Skip(APP_BRINGUP_MODULE_AUDIO_FRAME, 0);
    }
  }
  if (ret == TX_SUCCESS)
  {
    if (App_BringUpControl_IsEnabled(APP_BRINGUP_CONTROL_ACOUSTIC) != 0U)
    {
      AppAcousticImagingStatus_t acoustic_status = AppAcousticService_Init();
      if (acoustic_status == APP_ACOUSTIC_IMAGING_OK)
      {
        App_BringUpStatus_Ready(APP_BRINGUP_MODULE_ACOUSTIC, (int32_t)acoustic_status);
      }
      else
      {
        App_BringUpStatus_Fail(APP_BRINGUP_MODULE_ACOUSTIC, (int32_t)acoustic_status);
      }
      ret = tx_thread_create(&app_acoustic_thread,
                             "app_acoustic",
                             AppAcousticService_ThreadEntry,
                             0U,
                             app_acoustic_thread_stack,
                             APP_ACOUSTIC_THREAD_STACK_SIZE,
                             APP_ACOUSTIC_THREAD_PRIORITY,
                             APP_ACOUSTIC_THREAD_PRIORITY,
                             TX_NO_TIME_SLICE,
                             TX_AUTO_START);
      if (ret != TX_SUCCESS)
      {
        App_BringUpStatus_Fail(APP_BRINGUP_MODULE_ACOUSTIC, (int32_t)ret);
      }
    }
    else
    {
      App_BringUpStatus_Skip(APP_BRINGUP_MODULE_ACOUSTIC, 0);
    }
  }
  if (ret == TX_SUCCESS)
  {
    if (App_BringUpControl_IsEnabled(APP_BRINGUP_CONTROL_MEDIA) != 0U)
    {
      ret = AppMedia_Init(memory_ptr);
      if (ret == TX_SUCCESS)
      {
        App_BringUpStatus_Ready(APP_BRINGUP_MODULE_MEDIA, (int32_t)ret);
      }
      else
      {
        App_BringUpStatus_Fail(APP_BRINGUP_MODULE_MEDIA, (int32_t)ret);
      }
    }
    else
    {
      App_BringUpStatus_Skip(APP_BRINGUP_MODULE_MEDIA, 0);
    }
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
