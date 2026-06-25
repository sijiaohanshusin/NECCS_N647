/*
 * PCMD3180 board-level I2C/reset bus helpers.
 */

#include "app_pcmd_bus.h"

#include <string.h>

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
  uint8_t configured;
} AppPcmdBusConfig_t;

static AppPcmdBusConfig_t g_app_pcmd_bus_config;
static PCMD3180_BusTypeDef g_app_pcmd_bus;
static PCMD3180_HAL_BusContextTypeDef g_app_pcmd_bus_context;

void App_PCMD_BusInit(I2C_HandleTypeDef *hi2c,
                      GPIO_TypeDef *scl_port,
                      uint16_t scl_pin,
                      GPIO_TypeDef *sda_port,
                      uint16_t sda_pin,
                      GPIO_TypeDef *shutdown_port,
                      uint16_t shutdown_pin,
                      uint32_t timeout_ms)
{
  memset(&g_app_pcmd_bus_config, 0, sizeof(g_app_pcmd_bus_config));
  g_app_pcmd_bus_config.hi2c = hi2c;
  g_app_pcmd_bus_config.scl_port = scl_port;
  g_app_pcmd_bus_config.scl_pin = scl_pin;
  g_app_pcmd_bus_config.sda_port = sda_port;
  g_app_pcmd_bus_config.sda_pin = sda_pin;
  g_app_pcmd_bus_config.shutdown_port = shutdown_port;
  g_app_pcmd_bus_config.shutdown_pin = shutdown_pin;
  g_app_pcmd_bus_config.timeout_ms = timeout_ms;
  g_app_pcmd_bus_config.configured = 1U;
}

void App_PCMD_BusPrepare(uint8_t reset_device,
                         uint32_t reset_low_ms,
                         uint32_t reset_settle_ms)
{
  if (g_app_pcmd_bus_config.configured == 0U)
  {
    return;
  }

  memset(&g_app_pcmd_bus_context, 0, sizeof(g_app_pcmd_bus_context));
  g_app_pcmd_bus_context.hi2c = g_app_pcmd_bus_config.hi2c;
  g_app_pcmd_bus_context.scl_port = g_app_pcmd_bus_config.scl_port;
  g_app_pcmd_bus_context.scl_pin = g_app_pcmd_bus_config.scl_pin;
  g_app_pcmd_bus_context.sda_port = g_app_pcmd_bus_config.sda_port;
  g_app_pcmd_bus_context.sda_pin = g_app_pcmd_bus_config.sda_pin;
  g_app_pcmd_bus_context.shutdown_port = g_app_pcmd_bus_config.shutdown_port;
  g_app_pcmd_bus_context.shutdown_pin = g_app_pcmd_bus_config.shutdown_pin;
  g_app_pcmd_bus_context.timeout_ms = g_app_pcmd_bus_config.timeout_ms;

  /*
   * Match the validated H7 bring-up path: PCMD3180 control traffic is slow and
   * one-shot, so bit-banged I2C avoids HAL I2C state/IRQ interactions after
   * SAI clocks and DMA are running.
   */
  PCMD3180_HAL_BusInitSoftwareI2C(&g_app_pcmd_bus, &g_app_pcmd_bus_context);

  if ((reset_device != 0U) && (g_app_pcmd_bus.set_shutdown != NULL))
  {
    g_app_pcmd_bus.set_shutdown(g_app_pcmd_bus.context, 1U);
    HAL_Delay(reset_low_ms);
    g_app_pcmd_bus.set_shutdown(g_app_pcmd_bus.context, 0U);
    HAL_Delay(reset_settle_ms);
  }
}

PCMD3180_BusTypeDef *App_PCMD_BusGet(void)
{
  return &g_app_pcmd_bus;
}

PCMD3180_HAL_BusContextTypeDef *App_PCMD_BusGetContext(void)
{
  return &g_app_pcmd_bus_context;
}

uint8_t App_PCMD_BusRunAddressScan(uint8_t *ack_count,
                                   uint8_t *scan_rounds,
                                   uint8_t *scl_idle_high,
                                   uint8_t *sda_idle_high,
                                   uint8_t rounds)
{
  uint8_t all_stable = 1U;
  uint8_t local_scl_idle_high = 0U;
  uint8_t local_sda_idle_high = 0U;

  if ((ack_count == NULL) || (scan_rounds == NULL) ||
      (scl_idle_high == NULL) || (sda_idle_high == NULL) ||
      (rounds == 0U))
  {
    return 0U;
  }

  memset(ack_count, 0, PCMD3180_ARRAY_DEVICE_COUNT * sizeof(ack_count[0]));
  *scan_rounds = rounds;

  for (uint32_t round = 0U; round < rounds; round++)
  {
    for (uint32_t device = 0U;
         device < PCMD3180_ARRAY_DEVICE_COUNT;
         device++)
    {
      if (PCMD3180_HAL_ProbeAddress(&g_app_pcmd_bus_context,
                                    (uint8_t)(PCMD3180_I2C_ADDR_0 + device)) == PCMD3180_OK)
      {
        ack_count[device]++;
      }
    }
    HAL_Delay(1U);
  }

  PCMD3180_HAL_GetLineLevels(&g_app_pcmd_bus_context,
                             &local_scl_idle_high,
                             &local_sda_idle_high);
  *scl_idle_high = local_scl_idle_high;
  *sda_idle_high = local_sda_idle_high;

  for (uint32_t device = 0U;
       device < PCMD3180_ARRAY_DEVICE_COUNT;
       device++)
  {
    if (ack_count[device] != rounds)
    {
      all_stable = 0U;
    }
  }

  if ((local_scl_idle_high == 0U) || (local_sda_idle_high == 0U))
  {
    all_stable = 0U;
  }

  return all_stable;
}
