/*
 * STM32 HAL adapter for the PCMD3180 portable register driver.
 */

#ifndef PCMD3180_HAL_H
#define PCMD3180_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pcmd3180.h"
#include "stm32n6xx_hal.h"

typedef struct
{
    I2C_HandleTypeDef *hi2c;
    GPIO_TypeDef *scl_port;
    uint16_t scl_pin;
    GPIO_TypeDef *sda_port;
    uint16_t sda_pin;
    GPIO_TypeDef *shutdown_port;
    uint16_t shutdown_pin;
    uint32_t timeout_ms;
    uint32_t recover_count;
    uint32_t last_hal_status;
    uint32_t last_hal_error;
    uint8_t last_address7;
    uint8_t last_reg;
    uint8_t last_value;
    uint8_t last_is_read;
    uint8_t use_software_i2c;
} PCMD3180_HAL_BusContextTypeDef;

void PCMD3180_HAL_BusInit(PCMD3180_BusTypeDef *bus,
                          PCMD3180_HAL_BusContextTypeDef *context);

void PCMD3180_HAL_BusInitSoftwareI2C(PCMD3180_BusTypeDef *bus,
                                     PCMD3180_HAL_BusContextTypeDef *context);

PCMD3180_StatusTypeDef PCMD3180_HAL_WriteReg(void *context,
                                             uint8_t address7,
                                             uint8_t reg,
                                             uint8_t value);

PCMD3180_StatusTypeDef PCMD3180_HAL_ReadReg(void *context,
                                            uint8_t address7,
                                            uint8_t reg,
                                            uint8_t *value);

PCMD3180_StatusTypeDef PCMD3180_HAL_ProbeAddress(void *context,
                                                 uint8_t address7);

void PCMD3180_HAL_GetLineLevels(void *context,
                                uint8_t *scl_high,
                                uint8_t *sda_high);

void PCMD3180_HAL_DelayMs(void *context, uint32_t delay_ms);

void PCMD3180_HAL_SetShutdown(void *context, uint8_t asserted);

#ifdef __cplusplus
}
#endif

#endif /* PCMD3180_HAL_H */
