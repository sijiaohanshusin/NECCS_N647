#include "pcmd3180_hal.h"

#define PCMD3180_HAL_I2C_RETRY_COUNT       5U
#define PCMD3180_HAL_I2C_RETRY_DELAY_MS    5U
#define PCMD3180_HAL_I2C_BUS_CLEAR_PULSES  9U
#define PCMD3180_HAL_SW_I2C_HALF_PERIOD_US 20U

static uint8_t g_pcmd3180_dwt_delay_ready;

static uint32_t PCMD3180_HAL_GetTimeout(const PCMD3180_HAL_BusContextTypeDef *context)
{
    if ((context == NULL) || (context->timeout_ms == 0U))
    {
        return 100U;
    }

    return context->timeout_ms;
}

static HAL_StatusTypeDef PCMD3180_HAL_WaitForTransfer(
    PCMD3180_HAL_BusContextTypeDef *context)
{
    const uint32_t timeout_ms = PCMD3180_HAL_GetTimeout(context);
    const uint32_t start_tick = HAL_GetTick();

    while (HAL_I2C_GetState(context->hi2c) != HAL_I2C_STATE_READY)
    {
        if ((uint32_t)(HAL_GetTick() - start_tick) >= timeout_ms)
        {
            return HAL_TIMEOUT;
        }

        /* I2C/SAI/SysTick interrupts wake the core; I2C has top priority. */
        __WFI();
    }

    return (HAL_I2C_GetError(context->hi2c) == HAL_I2C_ERROR_NONE) ?
           HAL_OK : HAL_ERROR;
}

static uint32_t PCMD3180_HAL_SwI2CLock(void)
{
    const uint32_t primask = __get_PRIMASK();

    __disable_irq();
    __DSB();
    __ISB();
    return primask;
}

static void PCMD3180_HAL_SwI2CUnlock(uint32_t primask)
{
    __DSB();
    __ISB();
    __set_PRIMASK(primask);
}

static void PCMD3180_HAL_BusClear(PCMD3180_HAL_BusContextTypeDef *context)
{
    GPIO_InitTypeDef gpio_init = {0};

    if ((context == NULL) ||
        (context->scl_port == NULL) || (context->scl_pin == 0U) ||
        (context->sda_port == NULL) || (context->sda_pin == 0U))
    {
        return;
    }

    HAL_GPIO_WritePin(context->scl_port, context->scl_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(context->sda_port, context->sda_pin, GPIO_PIN_SET);

    gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;

    gpio_init.Pin = context->scl_pin;
    HAL_GPIO_Init(context->scl_port, &gpio_init);
    gpio_init.Pin = context->sda_pin;
    HAL_GPIO_Init(context->sda_port, &gpio_init);

    HAL_GPIO_WritePin(context->sda_port, context->sda_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(context->scl_port, context->scl_pin, GPIO_PIN_SET);
    HAL_Delay(1U);

    for (uint32_t pulse = 0U; pulse < PCMD3180_HAL_I2C_BUS_CLEAR_PULSES; pulse++)
    {
        HAL_GPIO_WritePin(context->scl_port, context->scl_pin, GPIO_PIN_RESET);
        HAL_Delay(1U);
        HAL_GPIO_WritePin(context->scl_port, context->scl_pin, GPIO_PIN_SET);
        HAL_Delay(1U);
    }

    /* Generate a STOP condition: SDA low while SCL is high, then release SDA. */
    HAL_GPIO_WritePin(context->sda_port, context->sda_pin, GPIO_PIN_RESET);
    HAL_Delay(1U);
    HAL_GPIO_WritePin(context->scl_port, context->scl_pin, GPIO_PIN_SET);
    HAL_Delay(1U);
    HAL_GPIO_WritePin(context->sda_port, context->sda_pin, GPIO_PIN_SET);
    HAL_Delay(1U);
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

    g_pcmd3180_dwt_delay_ready = (DWT->CYCCNT != 0U) ? 1U : 0U;
}

static void PCMD3180_HAL_SwDelayUs(uint32_t microseconds)
{
    if (g_pcmd3180_dwt_delay_ready != 0U)
    {
        const uint32_t cycles_per_us = SystemCoreClock / 1000000U;
        const uint32_t wait_cycles = cycles_per_us * microseconds;
        const uint32_t start_cycles = DWT->CYCCNT;

        while ((uint32_t)(DWT->CYCCNT - start_cycles) < wait_cycles)
        {
        }
        return;
    }

    /* Fallback only: the DWT counter is normally present on STM32N6. */
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

static void PCMD3180_HAL_SwSetScl(PCMD3180_HAL_BusContextTypeDef *context, GPIO_PinState state)
{
    HAL_GPIO_WritePin(context->scl_port, context->scl_pin, state);
    PCMD3180_HAL_SwDelay();
}

static void PCMD3180_HAL_SwSetSda(PCMD3180_HAL_BusContextTypeDef *context, GPIO_PinState state)
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

static uint8_t PCMD3180_HAL_SwWriteByte(PCMD3180_HAL_BusContextTypeDef *context, uint8_t data)
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

static uint8_t PCMD3180_HAL_SwReadByte(PCMD3180_HAL_BusContextTypeDef *context, uint8_t nack)
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

static void PCMD3180_HAL_SwI2CInitPins(PCMD3180_HAL_BusContextTypeDef *context)
{
    GPIO_InitTypeDef gpio_init = {0};

    if ((context == NULL) ||
        (context->scl_port == NULL) || (context->scl_pin == 0U) ||
        (context->sda_port == NULL) || (context->sda_pin == 0U))
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
    PCMD3180_HAL_SwSetScl(context, GPIO_PIN_SET);
    PCMD3180_HAL_SwSetSda(context, GPIO_PIN_SET);
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

static void PCMD3180_HAL_RecoverI2C(PCMD3180_HAL_BusContextTypeDef *context)
{
    I2C_HandleTypeDef *hi2c;

    if ((context == NULL) || (context->hi2c == NULL))
    {
        return;
    }

    hi2c = context->hi2c;
    context->recover_count++;
    /*
     * A failed PCMD transaction can leave either the HAL state machine or the
     * physical bus stuck. Drop the peripheral, manually release the bus, then
     * restore the CubeMX I2C configuration before retrying.
     */
    (void)HAL_I2C_DeInit(hi2c);
    PCMD3180_HAL_BusClear(context);
    (void)HAL_I2C_Init(hi2c);
    (void)HAL_I2CEx_ConfigAnalogFilter(hi2c, I2C_ANALOGFILTER_ENABLE);
    (void)HAL_I2CEx_ConfigDigitalFilter(hi2c, 0);
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

void PCMD3180_HAL_BusInitSoftwareI2C(PCMD3180_BusTypeDef *bus,
                                     PCMD3180_HAL_BusContextTypeDef *context)
{
    if (bus == NULL)
    {
        return;
    }

    if (context != NULL)
    {
        context->use_software_i2c = 1U;
        PCMD3180_HAL_SwI2CInitPins(context);
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
    HAL_StatusTypeDef hal_status;
    uint32_t attempt;

    if (hal_context == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    hal_context->last_address7 = address7;
    hal_context->last_reg = reg;
    hal_context->last_value = value;
    hal_context->last_is_read = 0U;

    if (hal_context->use_software_i2c != 0U)
    {
        PCMD3180_StatusTypeDef sw_status;

        for (attempt = 0U; attempt < PCMD3180_HAL_I2C_RETRY_COUNT; attempt++)
        {
            const uint32_t primask = PCMD3180_HAL_SwI2CLock();

            sw_status = PCMD3180_HAL_SwWriteRegOnce(hal_context, address7, reg, value);
            PCMD3180_HAL_SwI2CUnlock(primask);

            if (sw_status == PCMD3180_OK)
            {
                hal_context->last_hal_status = 0U;
                hal_context->last_hal_error = 0U;
                return PCMD3180_OK;
            }

            hal_context->recover_count++;
            PCMD3180_HAL_BusClear(hal_context);
            HAL_Delay(PCMD3180_HAL_I2C_RETRY_DELAY_MS);
        }

        hal_context->last_hal_status = 1U;
        hal_context->last_hal_error = 0xA0U;
        return PCMD3180_IO_ERROR;
    }

    if (hal_context->hi2c == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    for (attempt = 0U; attempt < PCMD3180_HAL_I2C_RETRY_COUNT; attempt++)
    {
        hal_status = HAL_I2C_Mem_Write_IT(hal_context->hi2c,
                                          (uint16_t)(address7 << 1),
                                          reg,
                                          I2C_MEMADD_SIZE_8BIT,
                                          &value,
                                          1U);
        if (hal_status == HAL_OK)
        {
            hal_status = PCMD3180_HAL_WaitForTransfer(hal_context);
        }
        hal_context->last_hal_status = (uint32_t)hal_status;
        hal_context->last_hal_error = HAL_I2C_GetError(hal_context->hi2c);
        if (hal_status == HAL_OK)
        {
            return PCMD3180_OK;
        }

        HAL_Delay(PCMD3180_HAL_I2C_RETRY_DELAY_MS);
        PCMD3180_HAL_RecoverI2C(hal_context);
    }

    return PCMD3180_IO_ERROR;
}

PCMD3180_StatusTypeDef PCMD3180_HAL_ReadReg(void *context,
                                            uint8_t address7,
                                            uint8_t reg,
                                            uint8_t *value)
{
    PCMD3180_HAL_BusContextTypeDef *hal_context = (PCMD3180_HAL_BusContextTypeDef *)context;
    HAL_StatusTypeDef hal_status;
    uint32_t attempt;

    if ((hal_context == NULL) || (value == NULL))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    hal_context->last_address7 = address7;
    hal_context->last_reg = reg;
    hal_context->last_value = 0U;
    hal_context->last_is_read = 1U;

    if (hal_context->use_software_i2c != 0U)
    {
        PCMD3180_StatusTypeDef sw_status;

        for (attempt = 0U; attempt < PCMD3180_HAL_I2C_RETRY_COUNT; attempt++)
        {
            const uint32_t primask = PCMD3180_HAL_SwI2CLock();

            sw_status = PCMD3180_HAL_SwReadRegOnce(hal_context, address7, reg, value);
            PCMD3180_HAL_SwI2CUnlock(primask);

            if (sw_status == PCMD3180_OK)
            {
                hal_context->last_value = *value;
                hal_context->last_hal_status = 0U;
                hal_context->last_hal_error = 0U;
                return PCMD3180_OK;
            }

            hal_context->recover_count++;
            PCMD3180_HAL_BusClear(hal_context);
            HAL_Delay(PCMD3180_HAL_I2C_RETRY_DELAY_MS);
        }

        hal_context->last_hal_status = 1U;
        hal_context->last_hal_error = 0xA1U;
        return PCMD3180_IO_ERROR;
    }

    if (hal_context->hi2c == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    for (attempt = 0U; attempt < PCMD3180_HAL_I2C_RETRY_COUNT; attempt++)
    {
        hal_status = HAL_I2C_Mem_Read_IT(hal_context->hi2c,
                                         (uint16_t)(address7 << 1),
                                         reg,
                                         I2C_MEMADD_SIZE_8BIT,
                                         value,
                                         1U);
        if (hal_status == HAL_OK)
        {
            hal_status = PCMD3180_HAL_WaitForTransfer(hal_context);
        }
        hal_context->last_hal_status = (uint32_t)hal_status;
        hal_context->last_hal_error = HAL_I2C_GetError(hal_context->hi2c);
        if (hal_status == HAL_OK)
        {
            hal_context->last_value = *value;
            return PCMD3180_OK;
        }

        HAL_Delay(PCMD3180_HAL_I2C_RETRY_DELAY_MS);
        PCMD3180_HAL_RecoverI2C(hal_context);
    }

    return PCMD3180_IO_ERROR;
}

PCMD3180_StatusTypeDef PCMD3180_HAL_ProbeAddress(void *context,
                                                 uint8_t address7)
{
    PCMD3180_HAL_BusContextTypeDef *hal_context =
        (PCMD3180_HAL_BusContextTypeDef *)context;

    if (hal_context == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    if (hal_context->use_software_i2c != 0U)
    {
        uint8_t nack;
        const uint32_t primask = PCMD3180_HAL_SwI2CLock();

        PCMD3180_HAL_SwStart(hal_context);
        nack = PCMD3180_HAL_SwWriteByte(hal_context, (uint8_t)(address7 << 1));
        PCMD3180_HAL_SwStop(hal_context);
        PCMD3180_HAL_SwI2CUnlock(primask);
        return (nack == 0U) ? PCMD3180_OK : PCMD3180_IO_ERROR;
    }

    if (hal_context->hi2c == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    return (HAL_I2C_IsDeviceReady(hal_context->hi2c,
                                  (uint16_t)(address7 << 1),
                                  1U,
                                  PCMD3180_HAL_GetTimeout(hal_context)) == HAL_OK) ?
           PCMD3180_OK : PCMD3180_IO_ERROR;
}

void PCMD3180_HAL_GetLineLevels(void *context,
                                uint8_t *scl_high,
                                uint8_t *sda_high)
{
    PCMD3180_HAL_BusContextTypeDef *hal_context =
        (PCMD3180_HAL_BusContextTypeDef *)context;

    if (scl_high != NULL)
    {
        *scl_high = 0U;
    }
    if (sda_high != NULL)
    {
        *sda_high = 0U;
    }
    if (hal_context == NULL)
    {
        return;
    }

    if ((scl_high != NULL) &&
        (hal_context->scl_port != NULL) && (hal_context->scl_pin != 0U))
    {
        *scl_high = (HAL_GPIO_ReadPin(hal_context->scl_port,
                                     hal_context->scl_pin) == GPIO_PIN_SET) ? 1U : 0U;
    }
    if ((sda_high != NULL) &&
        (hal_context->sda_port != NULL) && (hal_context->sda_pin != 0U))
    {
        *sda_high = (HAL_GPIO_ReadPin(hal_context->sda_port,
                                     hal_context->sda_pin) == GPIO_PIN_SET) ? 1U : 0U;
    }
}

void PCMD3180_HAL_DelayMs(void *context, uint32_t delay_ms)
{
    (void)context;
    HAL_Delay(delay_ms);
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
