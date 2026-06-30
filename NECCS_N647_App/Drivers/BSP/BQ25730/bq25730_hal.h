/*
 * STM32 HAL adapter for the BQ25730 portable register driver.
 */

#ifndef BQ25730_HAL_H
#define BQ25730_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bq25730.h"
#include "stm32n6xx_hal.h"

typedef struct
{
    I2C_HandleTypeDef *hi2c;
    GPIO_TypeDef *prochot_port;
    uint16_t prochot_pin;
    GPIO_TypeDef *chrg_ok_port;
    uint16_t chrg_ok_pin;
    GPIO_TypeDef *otg_vap_port;
    uint16_t otg_vap_pin;
    GPIO_TypeDef *cmpout_port;
    uint16_t cmpout_pin;
    GPIO_TypeDef *pg_port;
    uint16_t pg_pin;
    uint32_t timeout_ms;
} BQ25730_HAL_BusContextTypeDef;

void BQ25730_HAL_BusInit(BQ25730_BusTypeDef *bus,
                         BQ25730_HAL_BusContextTypeDef *context);

BQ25730_StatusTypeDef BQ25730_HAL_WriteReg(void *context,
                                           uint8_t address7,
                                           uint8_t reg,
                                           const uint8_t *data,
                                           uint16_t length);

BQ25730_StatusTypeDef BQ25730_HAL_ReadReg(void *context,
                                          uint8_t address7,
                                          uint8_t reg,
                                          uint8_t *data,
                                          uint16_t length);

void BQ25730_HAL_DelayMs(void *context, uint32_t delay_ms);

void BQ25730_HAL_SetOtgVap(void *context, uint8_t enabled);

BQ25730_StatusTypeDef BQ25730_HAL_ReadPins(void *context, uint32_t *pin_state);

#ifdef __cplusplus
}
#endif

#endif /* BQ25730_HAL_H */
