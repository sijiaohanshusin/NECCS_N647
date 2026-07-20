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
#include "app_camera_display.h"
#include "app_i2c2_bus.h"
#include "app_media.h"
#include "app_pcmd_capture.h"
#include "app_usb_device.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Priority ladder (lower number = higher priority):
 *   4  pcmd_capture (boot) - PCMD config window; drops to 12 once streaming
 *   5  TouchGFX          - event driven, must stay fluid
 *  11  bringup           - init + 1 Hz polling
 *  12  pcmd_capture      - hard real-time audio, must never starve
 *  13  camdisp worker    - compose+overlay, time-sliced with acoustic
 *  13  acoustic (SRP)    - time-sliced with camdisp (2-tick round robin)
 *  12  media             - FileX, mostly blocked
 * SRP used to sit BELOW the camera worker (prio 14, TX_NO_TIME_SLICE):
 * with the overlay active the worker monopolized the level and SRP fell
 * from ~7 to ~3-4 fps (the "crosshair lags the sound" complaint). Now both
 * share priority 13 WITH a 1-tick (10 ms) time slice: the worker's ~2 ms
 * compose bursts still fit well inside a camera frame (waits at most one
 * slice), while SRP round-robins instead of starving. Do NOT raise SRP to
 * 12: that level round-robins with the streaming-state PCMD thread and a
 * 70 ms SRP burst there starves the 5.3 ms SAI half cadence (dropped
 * halves, board-measured on earlier experiments). The PCMD thread is
 * CREATED at priority 4 and demotes itself (app_pcmd_capture.c): during
 * the boot screen TouchGFX renders full-time, so a 12-priority thread
 * would never run its I2C config sequence. */
#define APP_BRINGUP_THREAD_STACK_SIZE  4096U
#define APP_BRINGUP_THREAD_PRIORITY    11U
#define APP_PCMD_THREAD_STACK_SIZE     8192U
#define APP_PCMD_THREAD_PRIORITY       4U
#define APP_ACOUSTIC_THREAD_STACK_SIZE 12288U
#define APP_ACOUSTIC_THREAD_PRIORITY   13U
#define APP_ACOUSTIC_THREAD_TIME_SLICE 1U
#define APP_CAMDISP_THREAD_STACK_SIZE  4096U
#define APP_CAMDISP_THREAD_PRIORITY    13U
#define APP_CAMDISP_THREAD_TIME_SLICE  1U

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
static TX_THREAD app_camdisp_thread;
static ULONG app_camdisp_thread_stack[APP_CAMDISP_THREAD_STACK_SIZE / sizeof(ULONG)];

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
    ret = AppCameraDisplay_WorkerInit();
    if (ret == TX_SUCCESS)
    {
      ret = tx_thread_create(&app_camdisp_thread,
                             "app_camdisp",
                             AppCameraDisplay_WorkerThreadEntry,
                             0U,
                             app_camdisp_thread_stack,
                             APP_CAMDISP_THREAD_STACK_SIZE,
                             APP_CAMDISP_THREAD_PRIORITY,
                             APP_CAMDISP_THREAD_PRIORITY,
                             APP_CAMDISP_THREAD_TIME_SLICE,
                             TX_AUTO_START);
    }
    if (ret != TX_SUCCESS)
    {
      App_BringUpStatus_Fail(APP_BRINGUP_MODULE_DISPLAY, (int32_t)ret);
    }
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
                             APP_ACOUSTIC_THREAD_TIME_SLICE,
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
  if (ret == TX_SUCCESS)
  {
    /* USB MSC service: waits for the SD capacity, then raises the D+
     * pullup. Depends on media only through AppMedia_UsbBlockCount. */
    ret = AppUsbDevice_Init();
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
