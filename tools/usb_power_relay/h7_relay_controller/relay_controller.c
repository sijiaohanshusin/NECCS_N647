#include "relay_controller.h"

static UsbPowerRelay_State s_state = USB_POWER_RELAY_STATE_ON;
static uint32_t s_cycle_count = 0U;

static GPIO_PinState Relay_InactiveLevel(void)
{
  return (RELAY_ACTIVE_LEVEL == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
}

static GPIO_PinState Relay_LevelForState(UsbPowerRelay_State state)
{
  return (state == USB_POWER_RELAY_STATE_OFF) ? RELAY_ACTIVE_LEVEL : Relay_InactiveLevel();
}

void UsbPowerRelay_Init(void)
{
  UsbPowerRelay_Set(USB_POWER_RELAY_STATE_ON);
}

void UsbPowerRelay_Set(UsbPowerRelay_State state)
{
  HAL_GPIO_WritePin(RELAY_GPIO_PORT, RELAY_GPIO_PIN, Relay_LevelForState(state));
  s_state = state;
}

void UsbPowerRelay_Cycle(uint32_t off_ms)
{
  if (off_ms == 0U)
  {
    off_ms = USB_POWER_CYCLE_OFF_MS;
  }

  UsbPowerRelay_Set(USB_POWER_RELAY_STATE_OFF);
  HAL_Delay(off_ms);
  UsbPowerRelay_Set(USB_POWER_RELAY_STATE_ON);
  s_cycle_count++;
}

UsbPowerRelay_State UsbPowerRelay_GetState(void)
{
  return s_state;
}

uint32_t UsbPowerRelay_GetCycleCount(void)
{
  return s_cycle_count;
}

const char *UsbPowerRelay_GetStateString(void)
{
  return (s_state == USB_POWER_RELAY_STATE_OFF) ? "USB_OFF" : "USB_ON";
}

UsbPowerRelay_CommandStatus UsbPowerRelay_ApplyByte(uint8_t byte)
{
  switch (byte)
  {
  case '0':
  case 'n':
  case 'N':
    UsbPowerRelay_Set(USB_POWER_RELAY_STATE_ON);
    return USB_POWER_RELAY_CMD_OK;

  case '1':
  case 'f':
  case 'F':
    UsbPowerRelay_Set(USB_POWER_RELAY_STATE_OFF);
    return USB_POWER_RELAY_CMD_OK;

  case 'r':
  case 'R':
    UsbPowerRelay_Cycle(USB_POWER_CYCLE_OFF_MS);
    return USB_POWER_RELAY_CMD_OK;

  case '?':
    return USB_POWER_RELAY_CMD_OK;

  case '\r':
  case '\n':
  case ' ':
  case '\t':
    return USB_POWER_RELAY_CMD_NONE;

  default:
    return USB_POWER_RELAY_CMD_ERROR;
  }
}
