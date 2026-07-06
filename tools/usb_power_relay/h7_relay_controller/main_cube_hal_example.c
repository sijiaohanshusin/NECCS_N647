/*
 * Example STM32H7 Cube HAL integration.
 *
 * Copy the pieces below into the old H7 project's main.c. Do not compile this
 * file directly unless your project already provides SystemClock_Config(),
 * MX_GPIO_Init(), MX_USART3_UART_Init(), Error_Handler(), and huart3.
 */

#include "main.h"
#include "relay_controller.h"
#include <stdio.h>
#include <string.h>

#ifndef RELAY_UART_HANDLE
extern UART_HandleTypeDef huart3;
#define RELAY_UART_HANDLE huart3
#endif

static uint8_t s_relay_rx_byte;

static void RelayUart_Write(const char *text)
{
  (void)HAL_UART_Transmit(&RELAY_UART_HANDLE,
                          (uint8_t *)text,
                          (uint16_t)strlen(text),
                          100U);
}

static void RelayUart_Report(void)
{
  char msg[64];
  int len = snprintf(msg,
                     sizeof(msg),
                     "STATE %s CYCLES %lu\r\n",
                     UsbPowerRelay_GetStateString(),
                     (unsigned long)UsbPowerRelay_GetCycleCount());
  if (len > 0)
  {
    (void)HAL_UART_Transmit(&RELAY_UART_HANDLE, (uint8_t *)msg, (uint16_t)len, 100U);
  }
}

void RelayUart_StartRx(void)
{
  (void)HAL_UART_Receive_IT(&RELAY_UART_HANDLE, &s_relay_rx_byte, 1U);
}

void RelayUart_HandleByte(uint8_t byte)
{
  UsbPowerRelay_CommandStatus status = UsbPowerRelay_ApplyByte(byte);

  if (status == USB_POWER_RELAY_CMD_OK)
  {
    RelayUart_Report();
  }
  else if (status == USB_POWER_RELAY_CMD_ERROR)
  {
    RelayUart_Write("ERR\r\n");
  }
}

/*
 * If the H7 project already has HAL_UART_RxCpltCallback(), merge only this
 * body into the existing callback instead of adding a duplicate function.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &RELAY_UART_HANDLE)
  {
    RelayUart_HandleByte(s_relay_rx_byte);
    RelayUart_StartRx();
  }
}

/*
 * Minimal main() shape for reference:
 *
 * int main(void)
 * {
 *   HAL_Init();
 *   SystemClock_Config();
 *   MX_GPIO_Init();
 *   MX_USART3_UART_Init();
 *
 *   UsbPowerRelay_Init();
 *   RelayUart_Write("H7 USB relay ready\r\n");
 *   RelayUart_Report();
 *   RelayUart_StartRx();
 *
 *   while (1)
 *   {
 *     HAL_Delay(1000U);
 *   }
 * }
 *
 * Default GPIO init snippet if CubeMX is not used:
 *
 * __HAL_RCC_GPIOB_CLK_ENABLE();
 * GPIO_InitTypeDef GPIO_InitStruct = {0};
 * GPIO_InitStruct.Pin = GPIO_PIN_1;
 * GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
 * GPIO_InitStruct.Pull = GPIO_NOPULL;
 * GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
 * HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
 */
