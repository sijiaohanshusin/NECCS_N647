#ifndef APP_PCMD_DEBUG_H
#define APP_PCMD_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32n6xx_hal.h"

void App_PCMD_DebugSetRamStatus(uint32_t init_ok, uint32_t test_ok);
void App_PCMD_DebugBusInit(I2C_HandleTypeDef *hi2c);
void App_PCMD_DebugStartDma(void);
void App_PCMD_DebugInitAfterSaiClock(void);
void App_PCMD_DebugTask(void);
uint8_t App_PCMD_DebugRenderTask(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_PCMD_DEBUG_H */
