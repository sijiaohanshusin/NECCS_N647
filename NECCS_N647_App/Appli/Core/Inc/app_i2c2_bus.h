#ifndef APP_I2C2_BUS_H
#define APP_I2C2_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tx_api.h"
#include <stdint.h>

UINT AppI2C2_BusInit(void);
uint8_t AppI2C2_Lock(uint32_t timeout_ms);
void AppI2C2_Unlock(void);

extern volatile uint32_t g_app_i2c2_hal_restore_count;
extern volatile uint32_t g_app_i2c2_hal_restore_status;

#ifdef __cplusplus
}
#endif

#endif /* APP_I2C2_BUS_H */
