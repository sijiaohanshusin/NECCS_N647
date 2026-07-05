#ifndef APP_I2C2_BUS_H
#define APP_I2C2_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tx_api.h"
#include <stdint.h>

typedef struct
{
  uint8_t initialized;
  uint8_t locked;
  uint16_t reserved0;
  uint32_t lock_count;
  uint32_t lock_fail_count;
  uint32_t unlock_count;
  uint32_t restore_count;
  uint32_t restore_status;
  uint32_t recover_request_count;
  uint32_t recover_last_status;
  uint32_t active_since_ms;
  uint32_t active_timeout_ms;
  uint32_t active_thread;
  uint32_t last_lock_ms;
  uint32_t last_hold_ms;
  uint32_t max_hold_ms;
  uint32_t last_owner_thread;
} AppI2C2Snapshot_t;

UINT AppI2C2_BusInit(void);
uint8_t AppI2C2_Lock(uint32_t timeout_ms);
void AppI2C2_Unlock(void);
void AppI2C2_RequestRecovery(uint32_t status);
void AppI2C2_GetSnapshot(AppI2C2Snapshot_t *snapshot);

extern volatile uint32_t g_app_i2c2_hal_restore_count;
extern volatile uint32_t g_app_i2c2_hal_restore_status;
extern volatile uint32_t g_app_i2c2_hal_recover_request_count;
extern volatile uint32_t g_app_i2c2_hal_recover_last_status;
extern volatile AppI2C2Snapshot_t g_app_i2c2_snapshot;

#ifdef __cplusplus
}
#endif

#endif /* APP_I2C2_BUS_H */
