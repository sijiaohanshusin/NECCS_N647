/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined ( __ICCARM__ )
#  define CMSE_NS_CALL  __cmse_nonsecure_call
#  define CMSE_NS_ENTRY __cmse_nonsecure_entry
#else
#  define CMSE_NS_CALL  __attribute((cmse_nonsecure_call))
#  define CMSE_NS_ENTRY __attribute((cmse_nonsecure_entry))
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32n6xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* Function pointer declaration in non-secure*/
#if defined ( __ICCARM__ )
typedef void (CMSE_NS_CALL *funcptr)(void);
#else
typedef void CMSE_NS_CALL (*funcptr)(void);
#endif

/* typedef for non-secure callback functions */
typedef funcptr funcptr_NS;

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
HAL_StatusTypeDef MX_SDMMC2_SD_Init(void);
void MX_XSPI1_Init(void);
void MX_XSPIM_Init(void);

/* USER CODE BEGIN EFP */
extern SD_HandleTypeDef hsd2;
extern I2C_HandleTypeDef hi2c2;

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EXP_GPIO_PQ6_Pin GPIO_PIN_6
#define EXP_GPIO_PQ6_GPIO_Port GPIOQ
#define EXP_GPIO_PC10_Pin GPIO_PIN_10
#define EXP_GPIO_PC10_GPIO_Port GPIOC
#define EXP_GPIO_PE13_Pin GPIO_PIN_13
#define EXP_GPIO_PE13_GPIO_Port GPIOE
#define EXP_GPIO_PE15_Pin GPIO_PIN_15
#define EXP_GPIO_PE15_GPIO_Port GPIOE
#define UCPD1_INT_Pin GPIO_PIN_10
#define UCPD1_INT_GPIO_Port GPIOD
#define EXP_ALT_ETH_PD1_Pin GPIO_PIN_1
#define EXP_ALT_ETH_PD1_GPIO_Port GPIOD
#define CTP_RST_Pin GPIO_PIN_8
#define CTP_RST_GPIO_Port GPIOE
#define LCD_BL_PWM_Pin GPIO_PIN_9
#define LCD_BL_PWM_GPIO_Port GPIOE
#define EXP_GPIO_PC11_Pin GPIO_PIN_11
#define EXP_GPIO_PC11_GPIO_Port GPIOC
#define MIC_SHDNZ_Pin GPIO_PIN_6
#define MIC_SHDNZ_GPIO_Port GPIOC
#define CAM_LED_EN_Pin GPIO_PIN_14
#define CAM_LED_EN_GPIO_Port GPIOE
#define EXP_GPIO_PE12_Pin GPIO_PIN_12
#define EXP_GPIO_PE12_GPIO_Port GPIOE
#define EXP_GPIO_PE7_Pin GPIO_PIN_7
#define EXP_GPIO_PE7_GPIO_Port GPIOE
#define BQ25730_CMPOUT_Pin GPIO_PIN_8
#define BQ25730_CMPOUT_GPIO_Port GPIOH
#define EXP_GPIO_PC12_Pin GPIO_PIN_12
#define EXP_GPIO_PC12_GPIO_Port GPIOC
#define EXP_GPIO_PC7_Pin GPIO_PIN_7
#define EXP_GPIO_PC7_GPIO_Port GPIOC
#define EXP_ALT_ETH_PD12_Pin GPIO_PIN_12
#define EXP_ALT_ETH_PD12_GPIO_Port GPIOD
#define BQ25730_OTG_VAP_Pin GPIO_PIN_13
#define BQ25730_OTG_VAP_GPIO_Port GPIOD
#define EXT_SMPS_MODE_Pin GPIO_PIN_7
#define EXT_SMPS_MODE_GPIO_Port GPIOH
#define EXP_GPIO_PQ3_Pin GPIO_PIN_3
#define EXP_GPIO_PQ3_GPIO_Port GPIOQ
#define CTP_INT_Pin GPIO_PIN_4
#define CTP_INT_GPIO_Port GPIOQ
#define EXP_GPIO_PQ5_Pin GPIO_PIN_5
#define EXP_GPIO_PQ5_GPIO_Port GPIOQ
#define CAM_EN_MODULE_Pin GPIO_PIN_0
#define CAM_EN_MODULE_GPIO_Port GPIOD
#define EXP_GPIO_PQ2_Pin GPIO_PIN_2
#define EXP_GPIO_PQ2_GPIO_Port GPIOQ
#define EXP_GPIO_PQ1_Pin GPIO_PIN_1
#define EXP_GPIO_PQ1_GPIO_Port GPIOQ
#define EXP_GPIO_PQ0_Pin GPIO_PIN_0
#define EXP_GPIO_PQ0_GPIO_Port GPIOQ
#define EXP_GPIO_PH2_Pin GPIO_PIN_2
#define EXP_GPIO_PH2_GPIO_Port GPIOH
#define BQ25730_PROCHOT_Pin GPIO_PIN_3
#define BQ25730_PROCHOT_GPIO_Port GPIOD
#define TAMP_Pin GPIO_PIN_13
#define TAMP_GPIO_Port GPIOC
#define BQ25730_CHRG_OK_Pin GPIO_PIN_5
#define BQ25730_CHRG_OK_GPIO_Port GPIOH
#define BQ25730_PG_Pin GPIO_PIN_3
#define BQ25730_PG_GPIO_Port GPIOB
#define USER2_Pin GPIO_PIN_2
#define USER2_GPIO_Port GPIOE
#define USER1_Pin GPIO_PIN_0
#define USER1_GPIO_Port GPIOE
#define LCD_NRST_Pin GPIO_PIN_1
#define LCD_NRST_GPIO_Port GPIOE
#define PWR_SD_EN_Pin GPIO_PIN_7
#define PWR_SD_EN_GPIO_Port GPIOQ
#define EXP_GPIO_PD11_Pin GPIO_PIN_11
#define EXP_GPIO_PD11_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_1
#define LED1_GPIO_Port GPIOO
#define EXP_GPIO_PG7_Pin GPIO_PIN_7
#define EXP_GPIO_PG7_GPIO_Port GPIOG
#define EXP_GPIO_PG6_Pin GPIO_PIN_6
#define EXP_GPIO_PG6_GPIO_Port GPIOG
#define EXP_ALT_ETH_PF10_Pin GPIO_PIN_10
#define EXP_ALT_ETH_PF10_GPIO_Port GPIOF
#define EXP_ALT_ETH_PF7_Pin GPIO_PIN_7
#define EXP_ALT_ETH_PF7_GPIO_Port GPIOF
#define EXP_GPIO_PF3_Pin GPIO_PIN_3
#define EXP_GPIO_PF3_GPIO_Port GPIOF
#define EXP_GPIO_PF5_Pin GPIO_PIN_5
#define EXP_GPIO_PF5_GPIO_Port GPIOF
#define EXP_GPIO_PG4_Pin GPIO_PIN_4
#define EXP_GPIO_PG4_GPIO_Port GPIOG
#define EXP_ALT_ETH_PF11_Pin GPIO_PIN_11
#define EXP_ALT_ETH_PF11_GPIO_Port GPIOF
#define EXP_GPIO_PF1_Pin GPIO_PIN_1
#define EXP_GPIO_PF1_GPIO_Port GPIOF
#define EXP_GPIO_PG14_Pin GPIO_PIN_14
#define EXP_GPIO_PG14_GPIO_Port GPIOG
#define BOOT1_Pin GPIO_PIN_6
#define BOOT1_GPIO_Port GPIOA
#define SD_DET_Pin GPIO_PIN_12
#define SD_DET_GPIO_Port GPION
#define EXP_GPIO_PG3_Pin GPIO_PIN_3
#define EXP_GPIO_PG3_GPIO_Port GPIOG
#define EXP_ALT_ETH_PF13_Pin GPIO_PIN_13
#define EXP_ALT_ETH_PF13_GPIO_Port GPIOF
#define SPI2_CS0_Pin GPIO_PIN_0
#define SPI2_CS0_GPIO_Port GPIOF
#define EXP_GPIO_PG15_Pin GPIO_PIN_15
#define EXP_GPIO_PG15_GPIO_Port GPIOG
#define EXP_GPIO_PG1_Pin GPIO_PIN_1
#define EXP_GPIO_PG1_GPIO_Port GPIOG
#define EXP_GPIO_PB1_Pin GPIO_PIN_1
#define EXP_GPIO_PB1_GPIO_Port GPIOB
#define EXP_ALT_ETH_PF12_Pin GPIO_PIN_12
#define EXP_ALT_ETH_PF12_GPIO_Port GPIOF
#define EXP_ALT_ETH_PG12_Pin GPIO_PIN_12
#define EXP_ALT_ETH_PG12_GPIO_Port GPIOG
#define LED2_Pin GPIO_PIN_10
#define LED2_GPIO_Port GPIOG
#define EXP_GPIO_PG2_Pin GPIO_PIN_2
#define EXP_GPIO_PG2_GPIO_Port GPIOG
#define UCPD1_VSENSE_Pin GPIO_PIN_7
#define UCPD1_VSENSE_GPIO_Port GPION
#define EXP_GPIO_PA12_Pin GPIO_PIN_12
#define EXP_GPIO_PA12_GPIO_Port GPIOA
#define EXP_ALT_ETH_PG11_Pin GPIO_PIN_11
#define EXP_ALT_ETH_PG11_GPIO_Port GPIOG
#define USB1_EN_Pin GPIO_PIN_4
#define USB1_EN_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
