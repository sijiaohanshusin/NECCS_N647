/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32n6xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
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
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */
#ifndef APP_PCMD_IRQ_PRIORITY
#define APP_PCMD_IRQ_PRIORITY 10U
#endif

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static uint32_t SAI1_client = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */
static void App_SAI_ConfigCircularRxDma(DMA_HandleTypeDef *dma,
                                        DMA_NodeTypeDef *node,
                                        DMA_QListTypeDef *list,
                                        DMA_Channel_TypeDef *instance,
                                        uint32_t request)
{
  DMA_NodeConfTypeDef node_config = {0};

  node_config.NodeType = DMA_GPDMA_LINEAR_NODE;
  node_config.Init.Request = request;
  node_config.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
  node_config.Init.Direction = DMA_PERIPH_TO_MEMORY;
  node_config.Init.SrcInc = DMA_SINC_FIXED;
  node_config.Init.DestInc = DMA_DINC_INCREMENTED;
  node_config.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_HALFWORD;
  node_config.Init.DestDataWidth = DMA_DEST_DATAWIDTH_HALFWORD;
  node_config.Init.SrcBurstLength = 1;
  node_config.Init.DestBurstLength = 1;
  node_config.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
  node_config.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
  node_config.Init.Mode = DMA_NORMAL;
  node_config.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
  node_config.TriggerConfig.TriggerSelection = GPDMA1_TRIGGER_GPDMA1_CH0_TCF;
  node_config.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
  node_config.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
  node_config.SrcSecure = DMA_CHANNEL_SRC_SEC;
  node_config.DestSecure = DMA_CHANNEL_DEST_SEC;

  if (HAL_DMAEx_List_BuildNode(&node_config, node) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMAEx_List_InsertNode(list, NULL, node) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMAEx_List_SetCircularMode(list) != HAL_OK)
  {
    Error_Handler();
  }

  dma->Instance = instance;
  dma->InitLinkedList.Priority = DMA_HIGH_PRIORITY;
  dma->InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
  dma->InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
  dma->InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
  dma->InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
  if (HAL_DMAEx_List_Init(dma) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMAEx_List_LinkQ(dma, list) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{

  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  /* System interrupt init*/

  HAL_PWREx_EnableVddIO2();
  HAL_PWREx_ConfigVddIORange(PWR_VDDIO2,PWR_VDDIO_RANGE_1V8);

  HAL_PWREx_EnableVddIO3();
  HAL_PWREx_ConfigVddIORange(PWR_VDDIO3,PWR_VDDIO_RANGE_1V8);

  HAL_PWREx_EnableVddIO4();
  HAL_PWREx_ConfigVddIORange(PWR_VDDIO4,PWR_VDDIO_RANGE_3V3);

  HAL_PWREx_EnableVddIO5();
  HAL_PWREx_ConfigVddIORange(PWR_VDDIO5,PWR_VDDIO_RANGE_3V3);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
  * @brief CRC MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hcrc: CRC handle pointer
  * @retval None
  */
void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc)
{
  if(hcrc->Instance==CRC)
  {
    /* USER CODE BEGIN CRC_MspInit 0 */

    /* USER CODE END CRC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_CRC_CLK_ENABLE();
    /* USER CODE BEGIN CRC_MspInit 1 */

    /* USER CODE END CRC_MspInit 1 */

  }

}

/**
  * @brief CRC MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hcrc: CRC handle pointer
  * @retval None
  */
void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc)
{
  if(hcrc->Instance==CRC)
  {
    /* USER CODE BEGIN CRC_MspDeInit 0 */

    /* USER CODE END CRC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CRC_CLK_DISABLE();
    /* USER CODE BEGIN CRC_MspDeInit 1 */

    /* USER CODE END CRC_MspDeInit 1 */
  }

}

/**
  * @brief DMA2D MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hdma2d: DMA2D handle pointer
  * @retval None
  */
void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* hdma2d)
{
  if(hdma2d->Instance==DMA2D)
  {
    /* USER CODE BEGIN DMA2D_MspInit 0 */

    /* USER CODE END DMA2D_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_DMA2D_CLK_ENABLE();
    /* USER CODE BEGIN DMA2D_MspInit 1 */

    /* USER CODE END DMA2D_MspInit 1 */

  }

}

/**
  * @brief DMA2D MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hdma2d: DMA2D handle pointer
  * @retval None
  */
void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* hdma2d)
{
  if(hdma2d->Instance==DMA2D)
  {
    /* USER CODE BEGIN DMA2D_MspDeInit 0 */

    /* USER CODE END DMA2D_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DMA2D_CLK_DISABLE();
    /* USER CODE BEGIN DMA2D_MspDeInit 1 */

    /* USER CODE END DMA2D_MspDeInit 1 */
  }

}

#ifdef HAL_JPEG_MODULE_ENABLED
/**
  * @brief JPEG MSP Initialization
  * @param hjpeg: JPEG handle pointer
  * @retval None
  */
void HAL_JPEG_MspInit(JPEG_HandleTypeDef* hjpeg)
{
  if(hjpeg->Instance==JPEG)
  {
    __HAL_RCC_JPEG_CLK_ENABLE();
    __HAL_RCC_JPEG_FORCE_RESET();
    __HAL_RCC_JPEG_RELEASE_RESET();
  }
}

/**
  * @brief JPEG MSP De-Initialization
  * @param hjpeg: JPEG handle pointer
  * @retval None
  */
void HAL_JPEG_MspDeInit(JPEG_HandleTypeDef* hjpeg)
{
  if(hjpeg->Instance==JPEG)
  {
    __HAL_RCC_JPEG_FORCE_RESET();
    __HAL_RCC_JPEG_RELEASE_RESET();
    __HAL_RCC_JPEG_CLK_DISABLE();
  }
}
#endif /* HAL_JPEG_MODULE_ENABLED */

/**
  * @brief I2C MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(hi2c->Instance==I2C2)
  {
    /* USER CODE BEGIN I2C2_MspInit 0 */

    /* USER CODE END I2C2_MspInit 0 */

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
    PeriphClkInitStruct.I2c2ClockSelection = RCC_I2C2CLKSOURCE_CLKP;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**I2C2 GPIO Configuration
    PD4     ------> I2C2_SDA
    PD14     ------> I2C2_SCL
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();
    /* USER CODE BEGIN I2C2_MspInit 1 */

    /* USER CODE END I2C2_MspInit 1 */
  }

}

/**
  * @brief I2C MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if(hi2c->Instance==I2C2)
  {
    /* USER CODE BEGIN I2C2_MspDeInit 0 */

    /* USER CODE END I2C2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C2_CLK_DISABLE();

    /**I2C2 GPIO Configuration
    PD4     ------> I2C2_SDA
    PD14     ------> I2C2_SCL
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_4|GPIO_PIN_14);

    /* USER CODE BEGIN I2C2_MspDeInit 1 */

    /* USER CODE END I2C2_MspDeInit 1 */
  }

}

/**
  * @brief LTDC MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hltdc: LTDC handle pointer
  * @retval None
  */
void HAL_LTDC_MspInit(LTDC_HandleTypeDef* hltdc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(hltdc->Instance==LTDC)
  {
    /* USER CODE BEGIN LTDC_MspInit 0 */

    /* USER CODE END LTDC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.LtdcClockSelection = RCC_LTDCCLKSOURCE_IC16;
    PeriphClkInitStruct.ICSelection[RCC_IC16].ClockSelection = RCC_ICCLKSOURCE_PLL1;
    PeriphClkInitStruct.ICSelection[RCC_IC16].ClockDivider = 30;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_LTDC_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    /**LTDC GPIO Configuration
    PB15     ------> LTDC_G4
    PH4     ------> LTDC_R4
    PF8     ------> LTDC_R6
    PF9     ------> LTDC_HSYNC
    PA9     ------> LTDC_B5
    PA1     ------> LTDC_G2
    PB11     ------> LTDC_G6
    PA15(JTDI)     ------> LTDC_R5
    PA10     ------> LTDC_B4
    PA5     ------> LTDC_CLK
    PB12     ------> LTDC_G5
    PG0     ------> LTDC_VSYNC
    PA11     ------> LTDC_B3
    PA2     ------> LTDC_B7
    PB4(NJTRST)     ------> LTDC_R3
    PG9     ------> LTDC_R7
    PA8     ------> LTDC_B6
    PG13     ------> LTDC_DE
    PA0     ------> LTDC_G3
    PB10     ------> LTDC_G7
    */
    GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_4
                          |GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_LCD;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_LCD;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_LCD;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_1|GPIO_PIN_15|GPIO_PIN_10
                          |GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_2|GPIO_PIN_8
                          |GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_LCD;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_LCD;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_LCD;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* USER CODE BEGIN LTDC_MspInit 1 */

    /* USER CODE END LTDC_MspInit 1 */

  }

}

/**
  * @brief LTDC MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hltdc: LTDC handle pointer
  * @retval None
  */
void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef* hltdc)
{
  if(hltdc->Instance==LTDC)
  {
    /* USER CODE BEGIN LTDC_MspDeInit 0 */

    /* USER CODE END LTDC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LTDC_CLK_DISABLE();

    /**LTDC GPIO Configuration
    PB15     ------> LTDC_G4
    PH4     ------> LTDC_R4
    PF8     ------> LTDC_R6
    PF9     ------> LTDC_HSYNC
    PA9     ------> LTDC_B5
    PA1     ------> LTDC_G2
    PB11     ------> LTDC_G6
    PA15(JTDI)     ------> LTDC_R5
    PA10     ------> LTDC_B4
    PA5     ------> LTDC_CLK
    PB12     ------> LTDC_G5
    PG0     ------> LTDC_VSYNC
    PA11     ------> LTDC_B3
    PA2     ------> LTDC_B7
    PB4(NJTRST)     ------> LTDC_R3
    PG9     ------> LTDC_R7
    PA8     ------> LTDC_B6
    PG13     ------> LTDC_DE
    PA0     ------> LTDC_G3
    PB10     ------> LTDC_G7
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_4
                          |GPIO_PIN_10);

    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_4);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_8|GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_1|GPIO_PIN_15|GPIO_PIN_10
                          |GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_2|GPIO_PIN_8
                          |GPIO_PIN_0);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0|GPIO_PIN_9|GPIO_PIN_13);

    /* USER CODE BEGIN LTDC_MspDeInit 1 */

    /* USER CODE END LTDC_MspDeInit 1 */
  }

}

/**
  * @brief SD MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hsd: SD handle pointer
  * @retval None
  */
void HAL_SD_MspInit(SD_HandleTypeDef* hsd)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(hsd->Instance==SDMMC2)
  {
    /* USER CODE BEGIN SDMMC2_MspInit 0 */

    /* USER CODE END SDMMC2_MspInit 0 */

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC2;
    PeriphClkInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_IC4;
    PeriphClkInitStruct.ICSelection[RCC_IC4].ClockSelection = RCC_ICCLKSOURCE_PLL1;
    PeriphClkInitStruct.ICSelection[RCC_IC4].ClockDivider = 6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_SDMMC2_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**SDMMC2 GPIO Configuration
    PC0     ------> SDMMC2_D2
    PC3     ------> SDMMC2_CMD
    PC4     ------> SDMMC2_D0
    PC5     ------> SDMMC2_D1
    PD2     ------> SDMMC2_CK
    PE4     ------> SDMMC2_D3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_SDMMC2;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_SDMMC2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_SDMMC2;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* USER CODE BEGIN SDMMC2_MspInit 1 */

    /* USER CODE END SDMMC2_MspInit 1 */
  }

}

/**
  * @brief SD MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hsd: SD handle pointer
  * @retval None
  */
void HAL_SD_MspDeInit(SD_HandleTypeDef* hsd)
{
  if(hsd->Instance==SDMMC2)
  {
    /* USER CODE BEGIN SDMMC2_MspDeInit 0 */

    /* USER CODE END SDMMC2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SDMMC2_CLK_DISABLE();

    /**SDMMC2 GPIO Configuration
    PC0     ------> SDMMC2_D2
    PC3     ------> SDMMC2_CMD
    PC4     ------> SDMMC2_D0
    PC5     ------> SDMMC2_D1
    PD2     ------> SDMMC2_CK
    PE4     ------> SDMMC2_D3
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_4);

    /* USER CODE BEGIN SDMMC2_MspDeInit 1 */

    /* USER CODE END SDMMC2_MspDeInit 1 */
  }

}

/**
  * @brief XSPI MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hxspi: XSPI handle pointer
  * @retval None
  */
void HAL_XSPI_MspInit(XSPI_HandleTypeDef* hxspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(hxspi->Instance==XSPI1)
  {
    /* USER CODE BEGIN XSPI1_MspInit 0 */

    /* USER CODE END XSPI1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_XSPI1;
    PeriphClkInitStruct.Xspi1ClockSelection = RCC_XSPI1CLKSOURCE_IC4;
    PeriphClkInitStruct.ICSelection[RCC_IC4].ClockSelection = RCC_ICCLKSOURCE_PLL1;
    PeriphClkInitStruct.ICSelection[RCC_IC4].ClockDivider = 6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_XSPIM_CLK_ENABLE();
    __HAL_RCC_XSPI1_CLK_ENABLE();

    __HAL_RCC_GPIOP_CLK_ENABLE();
    __HAL_RCC_GPIOO_CLK_ENABLE();
    /**XSPI1 GPIO Configuration
    PP7     ------> XSPIM_P1_IO7
    PP6     ------> XSPIM_P1_IO6
    PP0     ------> XSPIM_P1_IO0
    PP4     ------> XSPIM_P1_IO4
    PP1     ------> XSPIM_P1_IO1
    PP5     ------> XSPIM_P1_IO5
    PP3     ------> XSPIM_P1_IO3
    PP2     ------> XSPIM_P1_IO2
    PO5     ------> XSPIM_P1_NCLK
    PO2     ------> XSPIM_P1_DQS0
    PO0     ------> XSPIM_P1_NCS1
    PO4     ------> XSPIM_P1_CLK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_0|GPIO_PIN_4
                          |GPIO_PIN_1|GPIO_PIN_5|GPIO_PIN_3|GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_XSPIM_P1;
    HAL_GPIO_Init(GPIOP, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_XSPIM_P1;
    HAL_GPIO_Init(GPIOO, &GPIO_InitStruct);

    /* USER CODE BEGIN XSPI1_MspInit 1 */

    /* USER CODE END XSPI1_MspInit 1 */

  }

}

/**
  * @brief XSPI MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hxspi: XSPI handle pointer
  * @retval None
  */
void HAL_XSPI_MspDeInit(XSPI_HandleTypeDef* hxspi)
{
  if(hxspi->Instance==XSPI1)
  {
    /* USER CODE BEGIN XSPI1_MspDeInit 0 */

    /* USER CODE END XSPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_XSPIM_CLK_DISABLE();
    __HAL_RCC_XSPI1_CLK_DISABLE();

    /**XSPI1 GPIO Configuration
    PP7     ------> XSPIM_P1_IO7
    PP6     ------> XSPIM_P1_IO6
    PP0     ------> XSPIM_P1_IO0
    PP4     ------> XSPIM_P1_IO4
    PP1     ------> XSPIM_P1_IO1
    PP5     ------> XSPIM_P1_IO5
    PP3     ------> XSPIM_P1_IO3
    PP2     ------> XSPIM_P1_IO2
    PO5     ------> XSPIM_P1_NCLK
    PO2     ------> XSPIM_P1_DQS0
    PO0     ------> XSPIM_P1_NCS1
    PO4     ------> XSPIM_P1_CLK
    */
    HAL_GPIO_DeInit(GPIOP, GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_0|GPIO_PIN_4
                          |GPIO_PIN_1|GPIO_PIN_5|GPIO_PIN_3|GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOO, GPIO_PIN_5|GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_4);

    /* USER CODE BEGIN XSPI1_MspDeInit 1 */

    /* USER CODE END XSPI1_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */
void HAL_DCMIPP_MspInit(DCMIPP_HandleTypeDef *hdcmipp)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  if (hdcmipp->Instance == DCMIPP)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_DCMIPP | RCC_PERIPHCLK_CSI;
    PeriphClkInitStruct.DcmippClockSelection = RCC_DCMIPPCLKSOURCE_IC17;
    PeriphClkInitStruct.ICSelection[RCC_IC17].ClockSelection = RCC_ICCLKSOURCE_PLL1;
    PeriphClkInitStruct.ICSelection[RCC_IC17].ClockDivider = 4;
    PeriphClkInitStruct.ICSelection[RCC_IC18].ClockSelection = RCC_ICCLKSOURCE_PLL1;
    PeriphClkInitStruct.ICSelection[RCC_IC18].ClockDivider = 60;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_DCMIPP_CLK_ENABLE();
    __HAL_RCC_CSI_CLK_ENABLE();

    __HAL_RCC_DCMIPP_FORCE_RESET();
    __HAL_RCC_DCMIPP_RELEASE_RESET();
    __HAL_RCC_CSI_FORCE_RESET();
    __HAL_RCC_CSI_RELEASE_RESET();

    HAL_NVIC_SetPriority(DCMIPP_IRQn, 5, 0);
    HAL_NVIC_SetPriority(CSI_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DCMIPP_IRQn);
    HAL_NVIC_EnableIRQ(CSI_IRQn);
  }
}

void HAL_DCMIPP_MspDeInit(DCMIPP_HandleTypeDef *hdcmipp)
{
  if (hdcmipp->Instance == DCMIPP)
  {
    __HAL_RCC_DCMIPP_FORCE_RESET();
    __HAL_RCC_DCMIPP_RELEASE_RESET();
    __HAL_RCC_CSI_FORCE_RESET();
    __HAL_RCC_CSI_RELEASE_RESET();

    __HAL_RCC_DCMIPP_CLK_DISABLE();
    __HAL_RCC_CSI_CLK_DISABLE();

    HAL_NVIC_DisableIRQ(DCMIPP_IRQn);
    HAL_NVIC_DisableIRQ(CSI_IRQn);
  }
}

void HAL_GPU2D_MspInit(GPU2D_HandleTypeDef* hgpu2d)
{
  if(hgpu2d->Instance==GPU2D)
  {
    __HAL_RCC_GPU2D_CLK_ENABLE();
    __HAL_RCC_GPU2D_FORCE_RESET();
    __HAL_RCC_GPU2D_RELEASE_RESET();
  }
}

void HAL_GPU2D_MspDeInit(GPU2D_HandleTypeDef* hgpu2d)
{
  if(hgpu2d->Instance==GPU2D)
  {
    __HAL_RCC_GPU2D_FORCE_RESET();
    __HAL_RCC_GPU2D_RELEASE_RESET();
    __HAL_RCC_GPU2D_CLK_DISABLE();
  }
}

void HAL_SAI_MspInit(SAI_HandleTypeDef* hsai)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  if (hsai->Instance == SAI1_Block_A)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_IC7;
    PeriphClkInitStruct.ICSelection[RCC_IC7].ClockSelection = RCC_ICCLKSOURCE_PLL2;
    PeriphClkInitStruct.ICSelection[RCC_IC7].ClockDivider = 20;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    if (SAI1_client == 0U)
    {
      __HAL_RCC_SAI1_CLK_ENABLE();
      HAL_NVIC_SetPriority(SAI1_A_IRQn, APP_PCMD_IRQ_PRIORITY, 0);
      HAL_NVIC_EnableIRQ(SAI1_A_IRQn);
      HAL_NVIC_SetPriority(SAI1_B_IRQn, APP_PCMD_IRQ_PRIORITY, 0);
      HAL_NVIC_EnableIRQ(SAI1_B_IRQn);
    }
    SAI1_client++;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    App_SAI_ConfigCircularRxDma(&handle_GPDMA1_Channel0,
                                &Node_GPDMA1_Channel0,
                                &List_GPDMA1_Channel0,
                                GPDMA1_Channel0,
                                GPDMA1_REQUEST_SAI1_A);

    __HAL_LINKDMA(hsai, hdmarx, handle_GPDMA1_Channel0);
  }

  if (hsai->Instance == SAI1_Block_B)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_IC7;
    PeriphClkInitStruct.ICSelection[RCC_IC7].ClockSelection = RCC_ICCLKSOURCE_PLL2;
    PeriphClkInitStruct.ICSelection[RCC_IC7].ClockDivider = 20;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    if (SAI1_client == 0U)
    {
      __HAL_RCC_SAI1_CLK_ENABLE();
      HAL_NVIC_SetPriority(SAI1_A_IRQn, APP_PCMD_IRQ_PRIORITY, 0);
      HAL_NVIC_EnableIRQ(SAI1_A_IRQn);
      HAL_NVIC_SetPriority(SAI1_B_IRQn, APP_PCMD_IRQ_PRIORITY, 0);
      HAL_NVIC_EnableIRQ(SAI1_B_IRQn);
    }
    SAI1_client++;

    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    App_SAI_ConfigCircularRxDma(&handle_GPDMA1_Channel1,
                                &Node_GPDMA1_Channel1,
                                &List_GPDMA1_Channel1,
                                GPDMA1_Channel1,
                                GPDMA1_REQUEST_SAI1_B);

    __HAL_LINKDMA(hsai, hdmarx, handle_GPDMA1_Channel1);
  }
}

void HAL_SAI_MspDeInit(SAI_HandleTypeDef* hsai)
{
  if (hsai->Instance == SAI1_Block_A)
  {
    if (SAI1_client > 0U)
    {
      SAI1_client--;
    }
    if (SAI1_client == 0U)
    {
      __HAL_RCC_SAI1_CLK_DISABLE();
      HAL_NVIC_DisableIRQ(SAI1_A_IRQn);
      HAL_NVIC_DisableIRQ(SAI1_B_IRQn);
    }

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_6);
    HAL_DMA_DeInit(hsai->hdmarx);
  }

  if (hsai->Instance == SAI1_Block_B)
  {
    if (SAI1_client > 0U)
    {
      SAI1_client--;
    }
    if (SAI1_client == 0U)
    {
      __HAL_RCC_SAI1_CLK_DISABLE();
      HAL_NVIC_DisableIRQ(SAI1_A_IRQn);
      HAL_NVIC_DisableIRQ(SAI1_B_IRQn);
    }

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_3);
    HAL_DMA_DeInit(hsai->hdmarx);
  }
}

/* USER CODE END 1 */
