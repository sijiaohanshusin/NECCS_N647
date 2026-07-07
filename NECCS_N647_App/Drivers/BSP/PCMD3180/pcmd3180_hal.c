#include "pcmd3180_hal.h"

#include "app_i2c2_bus.h"
#include "tx_api.h"

#ifndef PCMD3180_HAL_I2C_RETRY_COUNT
#define PCMD3180_HAL_I2C_RETRY_COUNT       5U
#endif

#ifndef PCMD3180_HAL_I2C_RETRY_DELAY_MS
#define PCMD3180_HAL_I2C_RETRY_DELAY_MS    5U
#endif

#ifndef PCMD3180_HAL_I2C_BUS_CLEAR_PULSES
#define PCMD3180_HAL_I2C_BUS_CLEAR_PULSES  9U
#endif

#ifndef PCMD3180_HAL_SW_I2C_HALF_PERIOD_US
#define PCMD3180_HAL_SW_I2C_HALF_PERIOD_US 20U
#endif

static uint8_t s_pcmd3180_dwt_delay_ready;

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

static void PCMD3180_HAL_SwDelayInit(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    for (volatile uint32_t probe = 0U; probe < 32U; probe++)
    {
        __NOP();
    }

    s_pcmd3180_dwt_delay_ready = (DWT->CYCCNT != 0U) ? 1U : 0U;
}

static void PCMD3180_HAL_SwDelayUs(uint32_t microseconds)
{
    if (s_pcmd3180_dwt_delay_ready != 0U)
    {
        const uint32_t cycles_per_us = SystemCoreClock / 1000000U;
        const uint32_t wait_cycles = cycles_per_us * microseconds;
        const uint32_t start_cycles = DWT->CYCCNT;

        while ((uint32_t)(DWT->CYCCNT - start_cycles) < wait_cycles)
        {
        }
        return;
    }

    volatile uint32_t delay = (SystemCoreClock / 5000000U) * microseconds;
    while (delay-- > 0U)
    {
        __NOP();
    }
}

static void PCMD3180_HAL_SwDelay(void)
{
    PCMD3180_HAL_SwDelayUs(PCMD3180_HAL_SW_I2C_HALF_PERIOD_US);
}

static uint8_t PCMD3180_HAL_SwPinsValid(const PCMD3180_HAL_BusContextTypeDef *context)
{
    return ((context != NULL) &&
            (context->scl_port != NULL) &&
            (context->scl_pin != 0U) &&
            (context->sda_port != NULL) &&
            (context->sda_pin != 0U)) ? 1U : 0U;
}

static void PCMD3180_HAL_SwI2CInitPins(PCMD3180_HAL_BusContextTypeDef *context)
{
    GPIO_InitTypeDef gpio_init = {0};

    if (PCMD3180_HAL_SwPinsValid(context) == 0U)
    {
        return;
    }

    if (context->hi2c != NULL)
    {
        (void)HAL_I2C_DeInit(context->hi2c);
    }

    HAL_GPIO_WritePin(context->scl_port, context->scl_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(context->sda_port, context->sda_pin, GPIO_PIN_SET);

    gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;

    gpio_init.Pin = context->scl_pin;
    HAL_GPIO_Init(context->scl_port, &gpio_init);
    gpio_init.Pin = context->sda_pin;
    HAL_GPIO_Init(context->sda_port, &gpio_init);

    PCMD3180_HAL_SwDelayInit();
    HAL_GPIO_WritePin(context->scl_port, context->scl_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(context->sda_port, context->sda_pin, GPIO_PIN_SET);
    PCMD3180_HAL_SwDelay();
}

static void PCMD3180_HAL_BusClear(PCMD3180_HAL_BusContextTypeDef *context)
{
    if (PCMD3180_HAL_SwPinsValid(context) == 0U)
    {
        return;
    }

    PCMD3180_HAL_SwI2CInitPins(context);
    for (uint32_t pulse = 0U; pulse < PCMD3180_HAL_I2C_BUS_CLEAR_PULSES; pulse++)
    {
        HAL_GPIO_WritePin(context->scl_port, context->scl_pin, GPIO_PIN_RESET);
        PCMD3180_HAL_SwDelayUs(1000U);
        HAL_GPIO_WritePin(context->scl_port, context->scl_pin, GPIO_PIN_SET);
        PCMD3180_HAL_SwDelayUs(1000U);
    }

    HAL_GPIO_WritePin(context->sda_port, context->sda_pin, GPIO_PIN_RESET);
    PCMD3180_HAL_SwDelayUs(1000U);
    HAL_GPIO_WritePin(context->scl_port, context->scl_pin, GPIO_PIN_SET);
    PCMD3180_HAL_SwDelayUs(1000U);
    HAL_GPIO_WritePin(context->sda_port, context->sda_pin, GPIO_PIN_SET);
    PCMD3180_HAL_SwDelayUs(1000U);
}

static uint32_t PCMD3180_HAL_SwI2CLockIrq(void)
{
    const uint32_t primask = __get_PRIMASK();

    __disable_irq();
    __DSB();
    __ISB();
    return primask;
}

static void PCMD3180_HAL_SwI2CUnlockIrq(uint32_t primask)
{
    __DSB();
    __ISB();
    __set_PRIMASK(primask);
}

static void PCMD3180_HAL_SwSetScl(PCMD3180_HAL_BusContextTypeDef *context,
                                  GPIO_PinState state)
{
    HAL_GPIO_WritePin(context->scl_port, context->scl_pin, state);
    PCMD3180_HAL_SwDelay();
}

static void PCMD3180_HAL_SwSetSda(PCMD3180_HAL_BusContextTypeDef *context,
                                  GPIO_PinState state)
{
    HAL_GPIO_WritePin(context->sda_port, context->sda_pin, state);
    PCMD3180_HAL_SwDelay();
}

static GPIO_PinState PCMD3180_HAL_SwReadSda(PCMD3180_HAL_BusContextTypeDef *context)
{
    PCMD3180_HAL_SwDelay();
    return HAL_GPIO_ReadPin(context->sda_port, context->sda_pin);
}

static void PCMD3180_HAL_SwStart(PCMD3180_HAL_BusContextTypeDef *context)
{
    PCMD3180_HAL_SwSetSda(context, GPIO_PIN_SET);
    PCMD3180_HAL_SwSetScl(context, GPIO_PIN_SET);
    PCMD3180_HAL_SwSetSda(context, GPIO_PIN_RESET);
    PCMD3180_HAL_SwSetScl(context, GPIO_PIN_RESET);
}

static void PCMD3180_HAL_SwStop(PCMD3180_HAL_BusContextTypeDef *context)
{
    PCMD3180_HAL_SwSetSda(context, GPIO_PIN_RESET);
    PCMD3180_HAL_SwSetScl(context, GPIO_PIN_SET);
    PCMD3180_HAL_SwSetSda(context, GPIO_PIN_SET);
}

static uint8_t PCMD3180_HAL_SwWriteByte(PCMD3180_HAL_BusContextTypeDef *context,
                                        uint8_t data)
{
    for (uint32_t bit = 0U; bit < 8U; bit++)
    {
        PCMD3180_HAL_SwSetSda(context, ((data & 0x80U) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        PCMD3180_HAL_SwSetScl(context, GPIO_PIN_SET);
        PCMD3180_HAL_SwSetScl(context, GPIO_PIN_RESET);
        data <<= 1;
    }

    PCMD3180_HAL_SwSetSda(context, GPIO_PIN_SET);
    PCMD3180_HAL_SwSetScl(context, GPIO_PIN_SET);
    const uint8_t nack = (PCMD3180_HAL_SwReadSda(context) == GPIO_PIN_SET) ? 1U : 0U;
    PCMD3180_HAL_SwSetScl(context, GPIO_PIN_RESET);
    return nack;
}

static uint8_t PCMD3180_HAL_SwReadByte(PCMD3180_HAL_BusContextTypeDef *context,
                                       uint8_t nack)
{
    uint8_t data = 0U;

    PCMD3180_HAL_SwSetSda(context, GPIO_PIN_SET);
    for (uint32_t bit = 0U; bit < 8U; bit++)
    {
        data <<= 1;
        PCMD3180_HAL_SwSetScl(context, GPIO_PIN_SET);
        if (PCMD3180_HAL_SwReadSda(context) == GPIO_PIN_SET)
        {
            data |= 1U;
        }
        PCMD3180_HAL_SwSetScl(context, GPIO_PIN_RESET);
    }

    PCMD3180_HAL_SwSetSda(context, (nack == 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    PCMD3180_HAL_SwSetScl(context, GPIO_PIN_SET);
    PCMD3180_HAL_SwSetScl(context, GPIO_PIN_RESET);
    PCMD3180_HAL_SwSetSda(context, GPIO_PIN_SET);
    return data;
}

static PCMD3180_StatusTypeDef PCMD3180_HAL_SwWriteRegOnce(PCMD3180_HAL_BusContextTypeDef *context,
                                                          uint8_t address7,
                                                          uint8_t reg,
                                                          uint8_t value)
{
    PCMD3180_HAL_SwStart(context);
    if (PCMD3180_HAL_SwWriteByte(context, (uint8_t)(address7 << 1)) != 0U)
    {
        PCMD3180_HAL_SwStop(context);
        return PCMD3180_IO_ERROR;
    }
    if (PCMD3180_HAL_SwWriteByte(context, reg) != 0U)
    {
        PCMD3180_HAL_SwStop(context);
        return PCMD3180_IO_ERROR;
    }
    if (PCMD3180_HAL_SwWriteByte(context, value) != 0U)
    {
        PCMD3180_HAL_SwStop(context);
        return PCMD3180_IO_ERROR;
    }

    PCMD3180_HAL_SwStop(context);
    return PCMD3180_OK;
}

static PCMD3180_StatusTypeDef PCMD3180_HAL_SwReadRegOnce(PCMD3180_HAL_BusContextTypeDef *context,
                                                         uint8_t address7,
                                                         uint8_t reg,
                                                         uint8_t *value)
{
    PCMD3180_HAL_SwStart(context);
    if (PCMD3180_HAL_SwWriteByte(context, (uint8_t)(address7 << 1)) != 0U)
    {
        PCMD3180_HAL_SwStop(context);
        return PCMD3180_IO_ERROR;
    }
    if (PCMD3180_HAL_SwWriteByte(context, reg) != 0U)
    {
        PCMD3180_HAL_SwStop(context);
        return PCMD3180_IO_ERROR;
    }

    PCMD3180_HAL_SwStart(context);
    if (PCMD3180_HAL_SwWriteByte(context, (uint8_t)((address7 << 1) | 0x01U)) != 0U)
    {
        PCMD3180_HAL_SwStop(context);
        return PCMD3180_IO_ERROR;
    }

    *value = PCMD3180_HAL_SwReadByte(context, 1U);
    PCMD3180_HAL_SwStop(context);
    return PCMD3180_OK;
}

static void PCMD3180_HAL_PrepareSoftwarePinsIfNeeded(PCMD3180_HAL_BusContextTypeDef *context)
{
    if ((context != NULL) && (context->software_i2c_active == 0U))
    {
        if ((context->hi2c != NULL) && (context->hi2c->Instance == I2C2))
        {
            (void)HAL_I2C_DeInit(context->hi2c);
        }
        PCMD3180_HAL_SwI2CInitPins(context);
    }
}

static void PCMD3180_HAL_RequestHalRestoreIfOneShot(PCMD3180_HAL_BusContextTypeDef *context)
{
    if ((context != NULL) &&
        (context->use_software_i2c != 0U) &&
        (context->software_i2c_active == 0U) &&
        (context->hi2c != NULL) &&
        (context->hi2c->Instance == I2C2))
    {
        AppI2C2_RequestRecovery((uint32_t)HAL_OK);
    }
}

static uint8_t PCMD3180_HAL_LockI2C(PCMD3180_HAL_BusContextTypeDef *context)
{
    if ((context != NULL) && (context->use_software_i2c != 0U) &&
        (context->software_i2c_active != 0U))
    {
        return 1U;
    }

    if ((context != NULL) && (context->hi2c != NULL) && (context->hi2c->Instance == I2C2))
    {
        return AppI2C2_Lock(PCMD3180_HAL_GetTimeout(context));
    }

    return 1U;
}

static void PCMD3180_HAL_UnlockI2C(PCMD3180_HAL_BusContextTypeDef *context)
{
    if ((context != NULL) && (context->use_software_i2c != 0U) &&
        (context->software_i2c_active != 0U))
    {
        return;
    }

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
    if (context != NULL)
    {
        context->use_software_i2c = 0U;
        context->software_i2c_active = 0U;
    }

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

void PCMD3180_HAL_SetSoftwareI2CEnabled(PCMD3180_HAL_BusContextTypeDef *context,
                                        uint8_t enabled)
{
    if (context == NULL)
    {
        return;
    }

    context->use_software_i2c = (enabled == 0U) ? 0U : 1U;
    if (enabled == 0U)
    {
        context->software_i2c_active = 0U;
    }
}

void PCMD3180_HAL_PrepareSoftwareI2C(PCMD3180_HAL_BusContextTypeDef *context)
{
    if ((context == NULL) || (context->use_software_i2c == 0U))
    {
        return;
    }

    if ((context->hi2c != NULL) && (context->hi2c->Instance == I2C2))
    {
        (void)HAL_I2C_DeInit(context->hi2c);
    }
    PCMD3180_HAL_SwI2CInitPins(context);
    context->software_i2c_active = 1U;
}

void PCMD3180_HAL_ReleaseSoftwareI2C(PCMD3180_HAL_BusContextTypeDef *context)
{
    if ((context == NULL) || (context->use_software_i2c == 0U))
    {
        return;
    }

    context->software_i2c_active = 0U;
    if ((context->hi2c != NULL) && (context->hi2c->Instance == I2C2))
    {
        AppI2C2_RequestRecovery((uint32_t)HAL_OK);
    }
}

PCMD3180_StatusTypeDef PCMD3180_HAL_WriteReg(void *context,
                                             uint8_t address7,
                                             uint8_t reg,
                                             uint8_t value)
{
    PCMD3180_HAL_BusContextTypeDef *hal_context = (PCMD3180_HAL_BusContextTypeDef *)context;
    HAL_StatusTypeDef hal_status = HAL_ERROR;

    if (hal_context == NULL)
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

        if (hal_context->use_software_i2c != 0U)
        {
            PCMD3180_StatusTypeDef sw_status;
            const uint32_t primask = PCMD3180_HAL_SwI2CLockIrq();

            PCMD3180_HAL_PrepareSoftwarePinsIfNeeded(hal_context);
            sw_status = PCMD3180_HAL_SwWriteRegOnce(hal_context, address7, reg, value);
            PCMD3180_HAL_SwI2CUnlockIrq(primask);
            if (sw_status == PCMD3180_OK)
            {
                hal_context->last_hal_status = (uint32_t)HAL_OK;
                hal_context->last_hal_error = 0U;
                PCMD3180_HAL_RequestHalRestoreIfOneShot(hal_context);
                PCMD3180_HAL_UnlockI2C(hal_context);
                return PCMD3180_OK;
            }
            hal_context->last_hal_status = (uint32_t)HAL_ERROR;
            hal_context->last_hal_error = 0xA0U;
            hal_context->recover_count++;
            PCMD3180_HAL_BusClear(hal_context);
            PCMD3180_HAL_RequestHalRestoreIfOneShot(hal_context);
            PCMD3180_HAL_UnlockI2C(hal_context);
            PCMD3180_HAL_Delay(PCMD3180_HAL_I2C_RETRY_DELAY_MS);
            continue;
        }

        if (hal_context->hi2c == NULL)
        {
            PCMD3180_HAL_UnlockI2C(hal_context);
            return PCMD3180_INVALID_ARGUMENT;
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

    if ((hal_context == NULL) || (value == NULL))
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

        if (hal_context->use_software_i2c != 0U)
        {
            PCMD3180_StatusTypeDef sw_status;
            const uint32_t primask = PCMD3180_HAL_SwI2CLockIrq();

            PCMD3180_HAL_PrepareSoftwarePinsIfNeeded(hal_context);
            sw_status = PCMD3180_HAL_SwReadRegOnce(hal_context, address7, reg, value);
            PCMD3180_HAL_SwI2CUnlockIrq(primask);
            if (sw_status == PCMD3180_OK)
            {
                hal_context->last_value = *value;
                hal_context->last_hal_status = (uint32_t)HAL_OK;
                hal_context->last_hal_error = 0U;
                PCMD3180_HAL_RequestHalRestoreIfOneShot(hal_context);
                PCMD3180_HAL_UnlockI2C(hal_context);
                return PCMD3180_OK;
            }
            hal_context->last_hal_status = (uint32_t)HAL_ERROR;
            hal_context->last_hal_error = 0xA1U;
            hal_context->recover_count++;
            PCMD3180_HAL_BusClear(hal_context);
            PCMD3180_HAL_RequestHalRestoreIfOneShot(hal_context);
            PCMD3180_HAL_UnlockI2C(hal_context);
            PCMD3180_HAL_Delay(PCMD3180_HAL_I2C_RETRY_DELAY_MS);
            continue;
        }

        if (hal_context->hi2c == NULL)
        {
            PCMD3180_HAL_UnlockI2C(hal_context);
            return PCMD3180_INVALID_ARGUMENT;
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
    HAL_StatusTypeDef hal_status = HAL_ERROR;

    if (hal_context == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    hal_context->last_address7 = address7;
    hal_context->last_reg = 0U;
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

        if (hal_context->use_software_i2c != 0U)
        {
            uint8_t nack;
            const uint32_t primask = PCMD3180_HAL_SwI2CLockIrq();

            PCMD3180_HAL_PrepareSoftwarePinsIfNeeded(hal_context);
            PCMD3180_HAL_SwStart(hal_context);
            nack = PCMD3180_HAL_SwWriteByte(hal_context, (uint8_t)(address7 << 1));
            PCMD3180_HAL_SwStop(hal_context);
            PCMD3180_HAL_SwI2CUnlockIrq(primask);
            if (nack == 0U)
            {
                hal_context->last_hal_status = (uint32_t)HAL_OK;
                hal_context->last_hal_error = 0U;
                PCMD3180_HAL_RequestHalRestoreIfOneShot(hal_context);
                PCMD3180_HAL_UnlockI2C(hal_context);
                return PCMD3180_OK;
            }
            hal_context->last_hal_status = (uint32_t)HAL_ERROR;
            hal_context->last_hal_error = 0xA2U;
            hal_context->recover_count++;
            PCMD3180_HAL_BusClear(hal_context);
            PCMD3180_HAL_RequestHalRestoreIfOneShot(hal_context);
            PCMD3180_HAL_UnlockI2C(hal_context);
            PCMD3180_HAL_Delay(PCMD3180_HAL_I2C_RETRY_DELAY_MS);
            continue;
        }

        if (hal_context->hi2c == NULL)
        {
            PCMD3180_HAL_UnlockI2C(hal_context);
            return PCMD3180_INVALID_ARGUMENT;
        }

        hal_status = HAL_I2C_IsDeviceReady(hal_context->hi2c,
                                           (uint16_t)(address7 << 1),
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
