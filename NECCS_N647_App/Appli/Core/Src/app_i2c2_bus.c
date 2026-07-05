#include "app_i2c2_bus.h"

#include "main.h"

#define APP_I2C2_TIMING 0x10707DBCUL

static TX_MUTEX g_app_i2c2_mutex;
static uint8_t g_app_i2c2_mutex_ready;
static volatile uint32_t g_app_i2c2_hal_recover_pending = 0U;

volatile uint32_t g_app_i2c2_hal_restore_count = 0U;
volatile uint32_t g_app_i2c2_hal_restore_status = HAL_OK;
volatile uint32_t g_app_i2c2_hal_recover_request_count = 0U;
volatile uint32_t g_app_i2c2_hal_recover_last_status = HAL_OK;
volatile AppI2C2Snapshot_t g_app_i2c2_snapshot;

static uint32_t AppI2C2_CurrentThreadToken(void)
{
  return (uint32_t)(uintptr_t)tx_thread_identify();
}

static HAL_StatusTypeDef AppI2C2_ReinitHal(void)
{
  HAL_StatusTypeDef status;

  if ((hi2c2.Instance == I2C2) && (hi2c2.State != HAL_I2C_STATE_RESET))
  {
    (void)HAL_I2C_DeInit(&hi2c2);
  }

  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = APP_I2C2_TIMING;
  hi2c2.Init.OwnAddress1 = 0U;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0U;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  status = HAL_I2C_Init(&hi2c2);
  if (status == HAL_OK)
  {
    status = HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE);
  }
  if (status == HAL_OK)
  {
    status = HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0U);
  }

  g_app_i2c2_hal_restore_count++;
  g_app_i2c2_hal_restore_status = status;
  g_app_i2c2_snapshot.restore_count = g_app_i2c2_hal_restore_count;
  g_app_i2c2_snapshot.restore_status = (uint32_t)status;
  if (status == HAL_OK)
  {
    g_app_i2c2_hal_recover_pending = 0U;
  }
  return status;
}

static HAL_StatusTypeDef AppI2C2_RestoreHalIfNeeded(void)
{
  if ((g_app_i2c2_hal_recover_pending == 0U) &&
      (hi2c2.Instance == I2C2) &&
      (hi2c2.State != HAL_I2C_STATE_RESET) &&
      ((I2C2->CR1 & I2C_CR1_PE) != 0U))
  {
    return HAL_OK;
  }

  return AppI2C2_ReinitHal();
}

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
    g_app_i2c2_snapshot.initialized = 1U;
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
  if (status == TX_SUCCESS)
  {
    g_app_i2c2_snapshot.lock_count++;
    g_app_i2c2_snapshot.locked = 1U;
    g_app_i2c2_snapshot.active_since_ms = HAL_GetTick();
    g_app_i2c2_snapshot.active_timeout_ms = timeout_ms;
    g_app_i2c2_snapshot.active_thread = AppI2C2_CurrentThreadToken();
    g_app_i2c2_snapshot.last_owner_thread = g_app_i2c2_snapshot.active_thread;
    if (AppI2C2_RestoreHalIfNeeded() != HAL_OK)
    {
      g_app_i2c2_snapshot.last_lock_ms = 0U;
      g_app_i2c2_snapshot.locked = 0U;
      g_app_i2c2_snapshot.active_thread = 0U;
      (void)tx_mutex_put(&g_app_i2c2_mutex);
      return 0U;
    }
    g_app_i2c2_snapshot.last_lock_ms = HAL_GetTick() - g_app_i2c2_snapshot.active_since_ms;
  }
  else
  {
    g_app_i2c2_snapshot.lock_fail_count++;
  }
  return (status == TX_SUCCESS) ? 1U : 0U;
}

void AppI2C2_Unlock(void)
{
  if (g_app_i2c2_mutex_ready != 0U)
  {
    uint32_t hold_ms = 0U;

    if (g_app_i2c2_snapshot.locked != 0U)
    {
      hold_ms = HAL_GetTick() - g_app_i2c2_snapshot.active_since_ms;
    }
    g_app_i2c2_snapshot.unlock_count++;
    g_app_i2c2_snapshot.last_hold_ms = hold_ms;
    if (hold_ms > g_app_i2c2_snapshot.max_hold_ms)
    {
      g_app_i2c2_snapshot.max_hold_ms = hold_ms;
    }
    g_app_i2c2_snapshot.locked = 0U;
    g_app_i2c2_snapshot.active_thread = 0U;
    (void)tx_mutex_put(&g_app_i2c2_mutex);
  }
}

void AppI2C2_RequestRecovery(uint32_t status)
{
  g_app_i2c2_hal_recover_last_status = status;
  g_app_i2c2_hal_recover_pending = 1U;
  ++g_app_i2c2_hal_recover_request_count;
  g_app_i2c2_snapshot.recover_last_status = status;
  g_app_i2c2_snapshot.recover_request_count = g_app_i2c2_hal_recover_request_count;
}

void AppI2C2_GetSnapshot(AppI2C2Snapshot_t *snapshot)
{
  uint32_t primask;

  if (snapshot == NULL)
  {
    return;
  }

  primask = __get_PRIMASK();
  __disable_irq();
  *snapshot = g_app_i2c2_snapshot;
  if (primask == 0U)
  {
    __enable_irq();
  }
}
