#include "bq25730_hal.h"
#include "app_i2c2_bus.h"

static uint32_t BQ25730_HAL_GetTimeout(const BQ25730_HAL_BusContextTypeDef *context)
{
    if ((context == NULL) || (context->timeout_ms == 0U))
    {
        return 100U;
    }

    return context->timeout_ms;
}

static void BQ25730_HAL_ReadPinIfPresent(GPIO_TypeDef *port,
                                         uint16_t pin,
                                         uint32_t mask,
                                         uint32_t *pin_state)
{
    if ((port != NULL) && (pin != 0U) && (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET))
    {
        *pin_state |= mask;
    }
}

void BQ25730_HAL_BusInit(BQ25730_BusTypeDef *bus,
                         BQ25730_HAL_BusContextTypeDef *context)
{
    if (bus == NULL)
    {
        return;
    }

    bus->context = context;
    bus->write_reg = BQ25730_HAL_WriteReg;
    bus->read_reg = BQ25730_HAL_ReadReg;
    bus->delay_ms = BQ25730_HAL_DelayMs;
    bus->set_otg_vap = BQ25730_HAL_SetOtgVap;
    bus->read_pins = BQ25730_HAL_ReadPins;
}

BQ25730_StatusTypeDef BQ25730_HAL_WriteReg(void *context,
                                           uint8_t address7,
                                           uint8_t reg,
                                           const uint8_t *data,
                                           uint16_t length)
{
    BQ25730_HAL_BusContextTypeDef *hal_context = (BQ25730_HAL_BusContextTypeDef *)context;
    HAL_StatusTypeDef hal_status;

    if ((hal_context == NULL) ||
        (hal_context->hi2c == NULL) ||
        (data == NULL) ||
        (length == 0U))
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    if (AppI2C2_Lock(BQ25730_HAL_GetTimeout(hal_context)) == 0U)
    {
        return BQ25730_IO_ERROR;
    }

    hal_status = HAL_I2C_Mem_Write(hal_context->hi2c,
                                   (uint16_t)(address7 << 1),
                                   reg,
                                   I2C_MEMADD_SIZE_8BIT,
                                   (uint8_t *)data,
                                   length,
                                   BQ25730_HAL_GetTimeout(hal_context));
    AppI2C2_Unlock();

    return (hal_status == HAL_OK) ? BQ25730_OK : BQ25730_IO_ERROR;
}

BQ25730_StatusTypeDef BQ25730_HAL_ReadReg(void *context,
                                          uint8_t address7,
                                          uint8_t reg,
                                          uint8_t *data,
                                          uint16_t length)
{
    BQ25730_HAL_BusContextTypeDef *hal_context = (BQ25730_HAL_BusContextTypeDef *)context;
    HAL_StatusTypeDef hal_status;

    if ((hal_context == NULL) ||
        (hal_context->hi2c == NULL) ||
        (data == NULL) ||
        (length == 0U))
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    if (AppI2C2_Lock(BQ25730_HAL_GetTimeout(hal_context)) == 0U)
    {
        return BQ25730_IO_ERROR;
    }

    hal_status = HAL_I2C_Mem_Read(hal_context->hi2c,
                                  (uint16_t)(address7 << 1),
                                  reg,
                                  I2C_MEMADD_SIZE_8BIT,
                                  data,
                                  length,
                                  BQ25730_HAL_GetTimeout(hal_context));
    AppI2C2_Unlock();

    return (hal_status == HAL_OK) ? BQ25730_OK : BQ25730_IO_ERROR;
}

void BQ25730_HAL_DelayMs(void *context, uint32_t delay_ms)
{
    (void)context;
    HAL_Delay(delay_ms);
}

void BQ25730_HAL_SetOtgVap(void *context, uint8_t enabled)
{
    BQ25730_HAL_BusContextTypeDef *hal_context = (BQ25730_HAL_BusContextTypeDef *)context;

    if ((hal_context == NULL) ||
        (hal_context->otg_vap_port == NULL) ||
        (hal_context->otg_vap_pin == 0U))
    {
        return;
    }

    HAL_GPIO_WritePin(hal_context->otg_vap_port,
                      hal_context->otg_vap_pin,
                      (enabled == 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

BQ25730_StatusTypeDef BQ25730_HAL_ReadPins(void *context, uint32_t *pin_state)
{
    BQ25730_HAL_BusContextTypeDef *hal_context = (BQ25730_HAL_BusContextTypeDef *)context;

    if ((hal_context == NULL) || (pin_state == NULL))
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    *pin_state = 0U;
    BQ25730_HAL_ReadPinIfPresent(hal_context->prochot_port,
                                 hal_context->prochot_pin,
                                 BQ25730_PIN_STATE_PROCHOT,
                                 pin_state);
    BQ25730_HAL_ReadPinIfPresent(hal_context->chrg_ok_port,
                                 hal_context->chrg_ok_pin,
                                 BQ25730_PIN_STATE_CHRG_OK,
                                 pin_state);
    BQ25730_HAL_ReadPinIfPresent(hal_context->otg_vap_port,
                                 hal_context->otg_vap_pin,
                                 BQ25730_PIN_STATE_OTG_VAP,
                                 pin_state);
    BQ25730_HAL_ReadPinIfPresent(hal_context->cmpout_port,
                                 hal_context->cmpout_pin,
                                 BQ25730_PIN_STATE_CMPOUT,
                                 pin_state);
    BQ25730_HAL_ReadPinIfPresent(hal_context->pg_port,
                                 hal_context->pg_pin,
                                 BQ25730_PIN_STATE_PG,
                                 pin_state);

    return BQ25730_OK;
}
