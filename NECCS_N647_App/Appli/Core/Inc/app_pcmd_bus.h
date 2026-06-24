/*
 * PCMD3180 board-level I2C/reset bus helpers.
 *
 * Keep this layer small: it owns the STM32 HAL bus context and address scans,
 * while the PCMD3180 portable driver owns register programming.
 */

#ifndef APP_PCMD_BUS_H
#define APP_PCMD_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "./PCMD3180/pcmd3180.h"
#include "./PCMD3180/pcmd3180_hal.h"

void App_PCMD_BusInit(I2C_HandleTypeDef *hi2c,
                      GPIO_TypeDef *scl_port,
                      uint16_t scl_pin,
                      GPIO_TypeDef *sda_port,
                      uint16_t sda_pin,
                      GPIO_TypeDef *shutdown_port,
                      uint16_t shutdown_pin,
                      uint32_t timeout_ms);

void App_PCMD_BusPrepare(uint8_t reset_device,
                         uint32_t reset_low_ms,
                         uint32_t reset_settle_ms);

PCMD3180_BusTypeDef *App_PCMD_BusGet(void);

PCMD3180_HAL_BusContextTypeDef *App_PCMD_BusGetContext(void);

uint8_t App_PCMD_BusRunAddressScan(uint8_t *ack_count,
                                   uint8_t *scan_rounds,
                                   uint8_t *scl_idle_high,
                                   uint8_t *sda_idle_high,
                                   uint8_t rounds);

#ifdef __cplusplus
}
#endif

#endif /* APP_PCMD_BUS_H */
