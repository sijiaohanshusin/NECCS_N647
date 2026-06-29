/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32n6xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "main.h"
#include "stm32n6xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_boot_diag.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint32_t g_app_systick_irq_count = 0;
volatile uint32_t g_app_threadx_systick_active = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern void _tx_timer_interrupt(void);

void App_ThreadX_SysTickHook(void)
{
  g_app_systick_irq_count++;
  HAL_IncTick();
  if (g_app_threadx_systick_active != 0U)
  {
    _tx_timer_interrupt();
  }
}

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */
extern DMA2D_HandleTypeDef hdma2d;
extern LTDC_HandleTypeDef hltdc;

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */
  App_BootDiag_RecordFault(APP_BOOT_STAGE_NMI);

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  App_BootDiag_RecordFault(APP_BOOT_STAGE_HARDFAULT);

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  App_BootDiag_RecordFault(APP_BOOT_STAGE_MEMMANAGE);

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */
  App_BootDiag_RecordFault(APP_BOOT_STAGE_BUSFAULT);

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */
  App_BootDiag_RecordFault(APP_BOOT_STAGE_USAGEFAULT);

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Secure fault.
  */
void SecureFault_Handler(void)
{
  /* USER CODE BEGIN SecureFault_IRQn 0 */
  App_BootDiag_RecordFault(APP_BOOT_STAGE_SECUREFAULT);

  /* USER CODE END SecureFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_SecureFault_IRQn 0 */
    /* USER CODE END W1_SecureFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32N6xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32n6xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA2D global interrupt.
  */
void DMA2D_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2D_IRQn 0 */
  g_app_boot_diag.dma2d_irq_count++;
  g_app_boot_diag.last_irqn = (uint32_t)DMA2D_IRQn;

  /* USER CODE END DMA2D_IRQn 0 */
  HAL_DMA2D_IRQHandler(&hdma2d);
  /* USER CODE BEGIN DMA2D_IRQn 1 */

  /* USER CODE END DMA2D_IRQn 1 */
}

/**
  * @brief This function handles LTDC up-layer global interrupt.
  */
void LTDC_UP_IRQHandler(void)
{
  /* USER CODE BEGIN LTDC_UP_IRQn 0 */
  g_app_boot_diag.ltdc_up_irq_count++;
  g_app_boot_diag.last_irqn = (uint32_t)LTDC_UP_IRQn;

  /* USER CODE END LTDC_UP_IRQn 0 */
  HAL_LTDC_IRQHandler(&hltdc);
  /* USER CODE BEGIN LTDC_UP_IRQn 1 */

  /* USER CODE END LTDC_UP_IRQn 1 */
}

/**
  * @brief This function handles LTDC up-layer error interrupt.
  */
void LTDC_UP_ERR_IRQHandler(void)
{
  /* USER CODE BEGIN LTDC_UP_ERR_IRQn 0 */
  g_app_boot_diag.ltdc_up_irq_count++;
  g_app_boot_diag.last_irqn = (uint32_t)LTDC_UP_ERR_IRQn;

  /* USER CODE END LTDC_UP_ERR_IRQn 0 */
  HAL_LTDC_IRQHandler(&hltdc);
  /* USER CODE BEGIN LTDC_UP_ERR_IRQn 1 */

  /* USER CODE END LTDC_UP_ERR_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
