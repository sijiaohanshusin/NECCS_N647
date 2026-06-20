#include "pcmd3180_hal.h"

static uint32_t PCMD3180_HAL_GetTimeout(const PCMD3180_HAL_BusContextTypeDef *context)
{
    if ((context == NULL) || (context->timeout_ms == 0U))
    {
        return 100U;
    }

    return context->timeout_ms;
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
    HAL_StatusTypeDef hal_status;

    if ((hal_context == NULL) || (hal_context->hi2c == NULL))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    hal_status = HAL_I2C_Mem_Write(hal_context->hi2c,
                                   (uint16_t)(address7 << 1),
                                   reg,
                                   I2C_MEMADD_SIZE_8BIT,
                                   &value,
                                   1U,
                                   PCMD3180_HAL_GetTimeout(hal_context));

    return (hal_status == HAL_OK) ? PCMD3180_OK : PCMD3180_IO_ERROR;
}

PCMD3180_StatusTypeDef PCMD3180_HAL_ReadReg(void *context,
                                            uint8_t address7,
                                            uint8_t reg,
                                            uint8_t *value)
{
    PCMD3180_HAL_BusContextTypeDef *hal_context = (PCMD3180_HAL_BusContextTypeDef *)context;
    HAL_StatusTypeDef hal_status;

    if ((hal_context == NULL) || (hal_context->hi2c == NULL) || (value == NULL))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    hal_status = HAL_I2C_Mem_Read(hal_context->hi2c,
                                  (uint16_t)(address7 << 1),
                                  reg,
                                  I2C_MEMADD_SIZE_8BIT,
                                  value,
                                  1U,
                                  PCMD3180_HAL_GetTimeout(hal_context));

    return (hal_status == HAL_OK) ? PCMD3180_OK : PCMD3180_IO_ERROR;
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
