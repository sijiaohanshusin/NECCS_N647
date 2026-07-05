#include "pcmd3180_hal.h"

#include "app_i2c2_bus.h"
#include "tx_api.h"

#ifndef PCMD3180_HAL_I2C_RETRY_COUNT
#define PCMD3180_HAL_I2C_RETRY_COUNT       2U
#endif

#ifndef PCMD3180_HAL_I2C_RETRY_DELAY_MS
#define PCMD3180_HAL_I2C_RETRY_DELAY_MS    2U
#endif

static uint32_t PCMD3180_HAL_GetTimeout(const PCMD3180_HAL_BusContextTypeDef *context)
{
    if ((context == NULL) || (context->timeout_ms == 0U))
    {
        return 100U;
    }

    return context->timeout_ms;
}

static ULONG PCMD3180_HAL_MsToTicks(uint32_t delay_ms)
{
    uint64_t ticks;

    if (delay_ms == 0U)
    {
        return 0U;
    }

    ticks = ((uint64_t)delay_ms * (uint64_t)TX_TIMER_TICKS_PER_SECOND) + 999ULL;
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

static void PCMD3180_HAL_Delay(uint32_t delay_ms)
{
    ULONG ticks;

    if (delay_ms == 0U)
    {
        return;
    }

    if (tx_thread_identify() == TX_NULL)
    {
        HAL_Delay(delay_ms);
        return;
    }

    ticks = PCMD3180_HAL_MsToTicks(delay_ms);
    if (ticks != 0U)
    {
        (void)tx_thread_sleep(ticks);
    }
}

static uint8_t PCMD3180_HAL_LockI2C(PCMD3180_HAL_BusContextTypeDef *context)
{
    if ((context != NULL) && (context->hi2c != NULL) && (context->hi2c->Instance == I2C2))
    {
        return AppI2C2_Lock(PCMD3180_HAL_GetTimeout(context));
    }

    return 1U;
}

static void PCMD3180_HAL_UnlockI2C(PCMD3180_HAL_BusContextTypeDef *context)
{
    if ((context != NULL) && (context->hi2c != NULL) && (context->hi2c->Instance == I2C2))
    {
        AppI2C2_Unlock();
    }
}

static void PCMD3180_HAL_RecordResult(PCMD3180_HAL_BusContextTypeDef *context,
                                      HAL_StatusTypeDef status)
{
    if (context == NULL)
    {
        return;
    }

    context->last_hal_status = (uint32_t)status;
    context->last_hal_error = (context->hi2c != NULL) ? HAL_I2C_GetError(context->hi2c) : 0U;
    if ((status != HAL_OK) &&
        (context->hi2c != NULL) &&
        (context->hi2c->Instance == I2C2))
    {
        AppI2C2_RequestRecovery((uint32_t)status);
    }
}

void PCMD3180_HAL_BusInit(PCMD3180_BusTypeDef *bus,
                          PCMD3180_HAL_BusContextTypeDef *context)
{
    if (bus == NULL)
    {
        return;
    }

    bus->context = context;
    bus->write_reg = PCMD3180_HAL_WriteReg;
    bus->read_reg = PCMD3180_HAL_ReadReg;
    bus->delay_ms = PCMD3180_HAL_DelayMs;
    bus->set_shutdown = PCMD3180_HAL_SetShutdown;
}

PCMD3180_StatusTypeDef PCMD3180_HAL_WriteReg(void *context,
                                             uint8_t address7,
                                             uint8_t reg,
                                             uint8_t value)
{
    PCMD3180_HAL_BusContextTypeDef *hal_context = (PCMD3180_HAL_BusContextTypeDef *)context;
    HAL_StatusTypeDef hal_status = HAL_ERROR;

    if ((hal_context == NULL) || (hal_context->hi2c == NULL))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    hal_context->last_address7 = address7;
    hal_context->last_reg = reg;
    hal_context->last_value = value;
    hal_context->last_is_read = 0U;

    for (uint32_t attempt = 0U; attempt < PCMD3180_HAL_I2C_RETRY_COUNT; attempt++)
    {
        if (PCMD3180_HAL_LockI2C(hal_context) == 0U)
        {
            hal_context->last_hal_status = (uint32_t)HAL_TIMEOUT;
            hal_context->last_hal_error = 0U;
            return PCMD3180_TIMEOUT;
        }

        hal_status = HAL_I2C_Mem_Write(hal_context->hi2c,
                                       (uint16_t)(address7 << 1),
                                       reg,
                                       I2C_MEMADD_SIZE_8BIT,
                                       &value,
                                       1U,
                                       PCMD3180_HAL_GetTimeout(hal_context));
        PCMD3180_HAL_RecordResult(hal_context, hal_status);
        PCMD3180_HAL_UnlockI2C(hal_context);
        if (hal_status == HAL_OK)
        {
            return PCMD3180_OK;
        }

        hal_context->recover_count++;
        PCMD3180_HAL_Delay(PCMD3180_HAL_I2C_RETRY_DELAY_MS);
    }

    return PCMD3180_IO_ERROR;
}

PCMD3180_StatusTypeDef PCMD3180_HAL_ReadReg(void *context,
                                            uint8_t address7,
                                            uint8_t reg,
                                            uint8_t *value)
{
    PCMD3180_HAL_BusContextTypeDef *hal_context = (PCMD3180_HAL_BusContextTypeDef *)context;
    HAL_StatusTypeDef hal_status = HAL_ERROR;

    if ((hal_context == NULL) || (hal_context->hi2c == NULL) || (value == NULL))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    hal_context->last_address7 = address7;
    hal_context->last_reg = reg;
    hal_context->last_value = 0U;
    hal_context->last_is_read = 1U;

    for (uint32_t attempt = 0U; attempt < PCMD3180_HAL_I2C_RETRY_COUNT; attempt++)
    {
        if (PCMD3180_HAL_LockI2C(hal_context) == 0U)
        {
            hal_context->last_hal_status = (uint32_t)HAL_TIMEOUT;
            hal_context->last_hal_error = 0U;
            return PCMD3180_TIMEOUT;
        }

        hal_status = HAL_I2C_Mem_Read(hal_context->hi2c,
                                      (uint16_t)(address7 << 1),
                                      reg,
                                      I2C_MEMADD_SIZE_8BIT,
                                      value,
                                      1U,
                                      PCMD3180_HAL_GetTimeout(hal_context));
        PCMD3180_HAL_RecordResult(hal_context, hal_status);
        PCMD3180_HAL_UnlockI2C(hal_context);
        if (hal_status == HAL_OK)
        {
            hal_context->last_value = *value;
            return PCMD3180_OK;
        }

        hal_context->recover_count++;
        PCMD3180_HAL_Delay(PCMD3180_HAL_I2C_RETRY_DELAY_MS);
    }

    return PCMD3180_IO_ERROR;
}

PCMD3180_StatusTypeDef PCMD3180_HAL_ProbeAddress(void *context,
                                                 uint8_t address7)
{
    PCMD3180_HAL_BusContextTypeDef *hal_context = (PCMD3180_HAL_BusContextTypeDef *)context;
    HAL_StatusTypeDef hal_status;

    if ((hal_context == NULL) || (hal_context->hi2c == NULL))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    hal_context->last_address7 = address7;
    hal_context->last_reg = 0U;
    hal_context->last_value = 0U;
    hal_context->last_is_read = 1U;

    if (PCMD3180_HAL_LockI2C(hal_context) == 0U)
    {
        hal_context->last_hal_status = (uint32_t)HAL_TIMEOUT;
        hal_context->last_hal_error = 0U;
        return PCMD3180_TIMEOUT;
    }

    hal_status = HAL_I2C_IsDeviceReady(hal_context->hi2c,
                                       (uint16_t)(address7 << 1),
                                       1U,
                                       PCMD3180_HAL_GetTimeout(hal_context));
    PCMD3180_HAL_RecordResult(hal_context, hal_status);
    PCMD3180_HAL_UnlockI2C(hal_context);
    return (hal_status == HAL_OK) ? PCMD3180_OK : PCMD3180_IO_ERROR;
}

void PCMD3180_HAL_DelayMs(void *context, uint32_t delay_ms)
{
    (void)context;
    PCMD3180_HAL_Delay(delay_ms);
}

void PCMD3180_HAL_SetShutdown(void *context, uint8_t asserted)
{
    PCMD3180_HAL_BusContextTypeDef *hal_context = (PCMD3180_HAL_BusContextTypeDef *)context;

    if ((hal_context == NULL) ||
        (hal_context->shutdown_port == NULL) ||
        (hal_context->shutdown_pin == 0U))
    {
        return;
    }

    HAL_GPIO_WritePin(hal_context->shutdown_port,
                      hal_context->shutdown_pin,
                      (asserted == 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
