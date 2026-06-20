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
    GPIO_TypeDef *shutdown_port;
    uint16_t shutdown_pin;
    uint32_t timeout_ms;
} PCMD3180_HAL_BusContextTypeDef;

void PCMD3180_HAL_BusInit(PCMD3180_BusTypeDef *bus,
                          PCMD3180_HAL_BusContextTypeDef *context);

PCMD3180_StatusTypeDef PCMD3180_HAL_WriteReg(void *context,
                                             uint8_t address7,
                                             uint8_t reg,
                                             uint8_t value);

PCMD3180_StatusTypeDef PCMD3180_HAL_ReadReg(void *context,
                                            uint8_t address7,
                                            uint8_t reg,
                                            uint8_t *value);

void PCMD3180_HAL_DelayMs(void *context, uint32_t delay_ms);

void PCMD3180_HAL_SetShutdown(void *context, uint8_t asserted);

#ifdef __cplusplus
}
#endif

#endif /* PCMD3180_HAL_H */
