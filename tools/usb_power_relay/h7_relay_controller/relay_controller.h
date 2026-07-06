#ifndef H7_USB_POWER_RELAY_CONTROLLER_H
#define H7_USB_POWER_RELAY_CONTROLLER_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RELAY_GPIO_PORT
#define RELAY_GPIO_PORT GPIOB
#endif

#ifndef RELAY_GPIO_PIN
#define RELAY_GPIO_PIN GPIO_PIN_1
#endif

/* Use GPIO_PIN_RESET for relay modules jumpered to low-level trigger. */
#ifndef RELAY_ACTIVE_LEVEL
#define RELAY_ACTIVE_LEVEL GPIO_PIN_SET
#endif

#ifndef USB_POWER_CYCLE_OFF_MS
#define USB_POWER_CYCLE_OFF_MS 5000U
#endif

typedef enum
{
  USB_POWER_RELAY_STATE_ON = 0,
  USB_POWER_RELAY_STATE_OFF = 1
} UsbPowerRelay_State;

typedef enum
{
  USB_POWER_RELAY_CMD_NONE = 0,
  USB_POWER_RELAY_CMD_OK,
  USB_POWER_RELAY_CMD_ERROR
} UsbPowerRelay_CommandStatus;

void UsbPowerRelay_Init(void);
void UsbPowerRelay_Set(UsbPowerRelay_State state);
void UsbPowerRelay_Cycle(uint32_t off_ms);
UsbPowerRelay_State UsbPowerRelay_GetState(void);
uint32_t UsbPowerRelay_GetCycleCount(void);
const char *UsbPowerRelay_GetStateString(void);
UsbPowerRelay_CommandStatus UsbPowerRelay_ApplyByte(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* H7_USB_POWER_RELAY_CONTROLLER_H */
