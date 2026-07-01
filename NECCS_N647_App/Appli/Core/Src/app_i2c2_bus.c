#include "app_i2c2_bus.h"

static TX_MUTEX g_app_i2c2_mutex;
static uint8_t g_app_i2c2_mutex_ready;

static ULONG timeout_ms_to_ticks(uint32_t timeout_ms)
{
  uint64_t ticks;

  if (timeout_ms == 0U)
  {
    return TX_WAIT_FOREVER;
  }

  ticks = ((uint64_t)timeout_ms * (uint64_t)TX_TIMER_TICKS_PER_SECOND) + 999ULL;
  ticks /= 1000ULL;
  if (ticks == 0ULL)
  {
    ticks = 1ULL;
  }
  if (ticks > 0xFFFFFFFFULL)
  {
    ticks = 0xFFFFFFFFULL;
  }

  return (ULONG)ticks;
}

UINT AppI2C2_BusInit(void)
{
  UINT status;

  if (g_app_i2c2_mutex_ready != 0U)
  {
    return TX_SUCCESS;
  }

  status = tx_mutex_create(&g_app_i2c2_mutex, (CHAR *)"i2c2_bus", TX_INHERIT);
  if (status == TX_SUCCESS)
  {
    g_app_i2c2_mutex_ready = 1U;
  }

  return status;
}

uint8_t AppI2C2_Lock(uint32_t timeout_ms)
{
  UINT status;

  if (g_app_i2c2_mutex_ready == 0U)
  {
    if (AppI2C2_BusInit() != TX_SUCCESS)
    {
      return 0U;
    }
  }

  status = tx_mutex_get(&g_app_i2c2_mutex, timeout_ms_to_ticks(timeout_ms));
  return (status == TX_SUCCESS) ? 1U : 0U;
}

void AppI2C2_Unlock(void)
{
  if (g_app_i2c2_mutex_ready != 0U)
  {
    (void)tx_mutex_put(&g_app_i2c2_mutex);
  }
}
