#include "bq25730.h"

#define BQ25730_CHARGE_CURRENT_STEP_MA       128U
#define BQ25730_CHARGE_CURRENT_MAX_MA        16256U
#define BQ25730_CHARGE_CURRENT_SHIFT         6U

#define BQ25730_CHARGE_VOLTAGE_STEP_MV       8U
#define BQ25730_CHARGE_VOLTAGE_MIN_MV        1024U
#define BQ25730_CHARGE_VOLTAGE_MAX_MV        23000U
#define BQ25730_CHARGE_VOLTAGE_SHIFT         3U

#define BQ25730_INPUT_CURRENT_STEP_MA        100U
#define BQ25730_INPUT_CURRENT_MAX_MA         12700U
#define BQ25730_CURRENT_LIMIT_SHIFT          8U

#define BQ25730_OTG_VOLTAGE_STEP_MV          8U
#define BQ25730_OTG_VOLTAGE_MIN_MV           3000U
#define BQ25730_OTG_VOLTAGE_MAX_MV           24000U
#define BQ25730_OTG_VOLTAGE_SHIFT            2U

#define BQ25730_OTG_CURRENT_STEP_MA          100U
#define BQ25730_OTG_CURRENT_MAX_MA           12700U

#define BQ25730_ADC_VBAT_VSYS_OFFSET_MV     2880U
#define BQ25730_ADC_VBAT_VSYS_STEP_MV       64U
#define BQ25730_ADC_ICHG_STEP_MA            128U
#define BQ25730_ADC_IDCHG_STEP_MA           512U
#define BQ25730_ADC_IIN_STEP_MA             100U
#define BQ25730_ADC_CMPIN_STEP_MV           12U

#define BQ25730_ADC_OPTION_ADC_CONV         0x8000U
#define BQ25730_ADC_OPTION_ADC_START        0x4000U
#define BQ25730_ADC_OPTION_ADC_FULLSCALE    0x2000U
#define BQ25730_ADC_OPTION_CHANNEL_MASK     0x00FFU

#define BQ25730_CHARGE_OPTION1_CMP_REF      0x0080U
#define BQ25730_CHARGE_OPTION1_CMP_POL      0x0040U
#define BQ25730_CHARGE_OPTION1_CMP_DEG_MASK 0x0030U
#define BQ25730_CHARGE_OPTION1_CMP_DEG_20MS 0x0020U
#define BQ25730_CHARGE_OPTION1_FORCE_CONV   0x0008U

#define BQ25730_CHARGE_OPTION3_EN_OTG        0x1000U
#define BQ25730_CHARGE_OPTION3_CMP_EN        0x0004U

static BQ25730_StatusTypeDef BQ25730_CheckHandle(const BQ25730_HandleTypeDef *handle)
{
    if ((handle == NULL) ||
        (handle->bus.write_reg == NULL) ||
        (handle->bus.read_reg == NULL) ||
        (handle->address7 > 0x7FU))
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    return BQ25730_OK;
}

static BQ25730_StatusTypeDef BQ25730_CheckRange(uint32_t value,
                                                uint32_t min_value,
                                                uint32_t max_value,
                                                uint8_t allow_zero)
{
    if ((allow_zero != 0U) && (value == 0U))
    {
        return BQ25730_OK;
    }

    if ((value < min_value) || (value > max_value))
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    return BQ25730_OK;
}

static BQ25730_StatusTypeDef BQ25730_UpdateRegister16(BQ25730_HandleTypeDef *handle,
                                                      uint8_t reg,
                                                      uint16_t clear_mask,
                                                      uint16_t set_mask)
{
    uint16_t value;
    BQ25730_StatusTypeDef status;

    status = BQ25730_ReadRegister16(handle, reg, &value);
    if (status != BQ25730_OK)
    {
        return status;
    }

    value &= (uint16_t)~clear_mask;
    value |= set_mask;

    return BQ25730_WriteRegister16(handle, reg, value);
}

BQ25730_StatusTypeDef BQ25730_Init(BQ25730_HandleTypeDef *handle,
                                   const BQ25730_BusTypeDef *bus,
                                   uint8_t address7)
{
    if ((handle == NULL) ||
        (bus == NULL) ||
        (bus->write_reg == NULL) ||
        (bus->read_reg == NULL) ||
        (address7 > 0x7FU))
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    handle->address7 = address7;
    handle->bus = *bus;

    return BQ25730_OK;
}

BQ25730_StatusTypeDef BQ25730_Probe(BQ25730_HandleTypeDef *handle,
                                    BQ25730_DeviceIdTypeDef *device_id)
{
    BQ25730_DeviceIdTypeDef id = {0U, 0U};
    BQ25730_StatusTypeDef status;

    status = BQ25730_ReadRegister8(handle, BQ25730_REG_MANUFACTURER_ID, &id.manufacturer_id);
    if (status != BQ25730_OK)
    {
        return status;
    }

    status = BQ25730_ReadRegister8(handle, BQ25730_REG_DEVICE_ID, &id.device_id);
    if (status != BQ25730_OK)
    {
        return status;
    }

    if (device_id != NULL)
    {
        *device_id = id;
    }

    if ((id.manufacturer_id != BQ25730_MANUFACTURER_ID_EXPECTED) ||
        (id.device_id != BQ25730_DEVICE_ID_EXPECTED))
    {
        return BQ25730_VERIFY_ERROR;
    }

    return BQ25730_OK;
}

BQ25730_StatusTypeDef BQ25730_WriteRegister8(BQ25730_HandleTypeDef *handle,
                                             uint8_t reg,
                                             uint8_t value)
{
    BQ25730_StatusTypeDef status;

    status = BQ25730_CheckHandle(handle);
    if (status != BQ25730_OK)
    {
        return status;
    }

    return handle->bus.write_reg(handle->bus.context,
                                 handle->address7,
                                 reg,
                                 &value,
                                 1U);
}

BQ25730_StatusTypeDef BQ25730_ReadRegister8(BQ25730_HandleTypeDef *handle,
                                            uint8_t reg,
                                            uint8_t *value)
{
    BQ25730_StatusTypeDef status;

    status = BQ25730_CheckHandle(handle);
    if (status != BQ25730_OK)
    {
        return status;
    }

    if (value == NULL)
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    return handle->bus.read_reg(handle->bus.context,
                                handle->address7,
                                reg,
                                value,
                                1U);
}

BQ25730_StatusTypeDef BQ25730_WriteRegister16(BQ25730_HandleTypeDef *handle,
                                              uint8_t reg_lsb,
                                              uint16_t value)
{
    uint8_t data[2];
    BQ25730_StatusTypeDef status;

    status = BQ25730_CheckHandle(handle);
    if (status != BQ25730_OK)
    {
        return status;
    }

    data[0] = (uint8_t)(value & 0x00FFU);
    data[1] = (uint8_t)((value >> 8U) & 0x00FFU);

    return handle->bus.write_reg(handle->bus.context,
                                 handle->address7,
                                 reg_lsb,
                                 data,
                                 2U);
}

BQ25730_StatusTypeDef BQ25730_ReadRegister16(BQ25730_HandleTypeDef *handle,
                                             uint8_t reg_lsb,
                                             uint16_t *value)
{
    uint8_t data[2] = {0U, 0U};
    BQ25730_StatusTypeDef status;

    status = BQ25730_CheckHandle(handle);
    if (status != BQ25730_OK)
    {
        return status;
    }

    if (value == NULL)
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    status = handle->bus.read_reg(handle->bus.context,
                                  handle->address7,
                                  reg_lsb,
                                  data,
                                  2U);
    if (status != BQ25730_OK)
    {
        return status;
    }

    *value = (uint16_t)data[0] | ((uint16_t)data[1] << 8U);
    return BQ25730_OK;
}

BQ25730_StatusTypeDef BQ25730_SetChargeCurrentMa(BQ25730_HandleTypeDef *handle,
                                                 uint32_t current_ma)
{
    uint32_t code;
    BQ25730_StatusTypeDef status;

    status = BQ25730_CheckRange(current_ma, 0U, BQ25730_CHARGE_CURRENT_MAX_MA, 1U);
    if (status != BQ25730_OK)
    {
        return status;
    }

    code = current_ma / BQ25730_CHARGE_CURRENT_STEP_MA;
    return BQ25730_WriteRegister16(handle,
                                   BQ25730_REG_CHARGE_CURRENT,
                                   (uint16_t)(code << BQ25730_CHARGE_CURRENT_SHIFT));
}

BQ25730_StatusTypeDef BQ25730_SetChargeVoltageMv(BQ25730_HandleTypeDef *handle,
                                                 uint32_t voltage_mv)
{
    uint32_t code;
    BQ25730_StatusTypeDef status;

    status = BQ25730_CheckRange(voltage_mv,
                                BQ25730_CHARGE_VOLTAGE_MIN_MV,
                                BQ25730_CHARGE_VOLTAGE_MAX_MV,
                                1U);
    if (status != BQ25730_OK)
    {
        return status;
    }

    code = voltage_mv / BQ25730_CHARGE_VOLTAGE_STEP_MV;
    return BQ25730_WriteRegister16(handle,
                                   BQ25730_REG_CHARGE_VOLTAGE,
                                   (uint16_t)(code << BQ25730_CHARGE_VOLTAGE_SHIFT));
}

BQ25730_StatusTypeDef BQ25730_SetInputCurrentLimitMa(BQ25730_HandleTypeDef *handle,
                                                     uint32_t current_ma)
{
    uint32_t code;
    BQ25730_StatusTypeDef status;

    status = BQ25730_CheckRange(current_ma, 0U, BQ25730_INPUT_CURRENT_MAX_MA, 1U);
    if (status != BQ25730_OK)
    {
        return status;
    }

    code = current_ma / BQ25730_INPUT_CURRENT_STEP_MA;
    return BQ25730_WriteRegister16(handle,
                                   BQ25730_REG_IIN_HOST,
                                   (uint16_t)(code << BQ25730_CURRENT_LIMIT_SHIFT));
}

BQ25730_StatusTypeDef BQ25730_SetOtgVoltageMv(BQ25730_HandleTypeDef *handle,
                                              uint32_t voltage_mv)
{
    uint32_t code;
    BQ25730_StatusTypeDef status;

    status = BQ25730_CheckRange(voltage_mv,
                                BQ25730_OTG_VOLTAGE_MIN_MV,
                                BQ25730_OTG_VOLTAGE_MAX_MV,
                                0U);
    if (status != BQ25730_OK)
    {
        return status;
    }

    code = voltage_mv / BQ25730_OTG_VOLTAGE_STEP_MV;
    return BQ25730_WriteRegister16(handle,
                                   BQ25730_REG_OTG_VOLTAGE,
                                   (uint16_t)(code << BQ25730_OTG_VOLTAGE_SHIFT));
}

BQ25730_StatusTypeDef BQ25730_SetOtgCurrentMa(BQ25730_HandleTypeDef *handle,
                                             uint32_t current_ma)
{
    uint32_t code;
    BQ25730_StatusTypeDef status;

    status = BQ25730_CheckRange(current_ma, 0U, BQ25730_OTG_CURRENT_MAX_MA, 1U);
    if (status != BQ25730_OK)
    {
        return status;
    }

    code = current_ma / BQ25730_OTG_CURRENT_STEP_MA;
    return BQ25730_WriteRegister16(handle,
                                   BQ25730_REG_OTG_CURRENT,
                                   (uint16_t)(code << BQ25730_CURRENT_LIMIT_SHIFT));
}

BQ25730_StatusTypeDef BQ25730_SetOtgEnabled(BQ25730_HandleTypeDef *handle,
                                            uint8_t enabled)
{
    uint16_t charge_option3;
    BQ25730_StatusTypeDef status;

    status = BQ25730_ReadRegister16(handle, BQ25730_REG_CHARGE_OPTION3, &charge_option3);
    if (status != BQ25730_OK)
    {
        return status;
    }

    if (enabled == 0U)
    {
        if (handle->bus.set_otg_vap != NULL)
        {
            handle->bus.set_otg_vap(handle->bus.context, 0U);
        }
        charge_option3 &= (uint16_t)~BQ25730_CHARGE_OPTION3_EN_OTG;
    }
    else
    {
        charge_option3 |= BQ25730_CHARGE_OPTION3_EN_OTG;
    }

    status = BQ25730_WriteRegister16(handle, BQ25730_REG_CHARGE_OPTION3, charge_option3);
    if (status != BQ25730_OK)
    {
        return status;
    }

    if ((enabled != 0U) && (handle->bus.set_otg_vap != NULL))
    {
        handle->bus.set_otg_vap(handle->bus.context, 1U);
    }

    return BQ25730_OK;
}

BQ25730_StatusTypeDef BQ25730_ReadChargerStatus(BQ25730_HandleTypeDef *handle,
                                                uint16_t *charger_status)
{
    return BQ25730_ReadRegister16(handle, BQ25730_REG_CHARGER_STATUS, charger_status);
}

BQ25730_StatusTypeDef BQ25730_ReadProchotStatus(BQ25730_HandleTypeDef *handle,
                                                uint16_t *prochot_status)
{
    return BQ25730_ReadRegister16(handle, BQ25730_REG_PROCHOT_STATUS, prochot_status);
}

BQ25730_StatusTypeDef BQ25730_ConfigureAdc(BQ25730_HandleTypeDef *handle,
                                           uint16_t channels,
                                           uint8_t continuous)
{
    uint16_t clear_mask;
    uint16_t set_mask;

    if ((channels & (uint16_t)~BQ25730_ADC_CHANNEL_ALL) != 0U)
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    clear_mask = BQ25730_ADC_OPTION_CHANNEL_MASK | BQ25730_ADC_OPTION_ADC_CONV;
    set_mask = BQ25730_ADC_OPTION_ADC_START |
               BQ25730_ADC_OPTION_ADC_FULLSCALE |
               (channels & BQ25730_ADC_OPTION_CHANNEL_MASK);

    if (continuous != 0U)
    {
        set_mask |= BQ25730_ADC_OPTION_ADC_CONV;
    }

    return BQ25730_UpdateRegister16(handle,
                                    BQ25730_REG_ADC_OPTION,
                                    clear_mask,
                                    set_mask);
}

BQ25730_StatusTypeDef BQ25730_ReadAdcRaw(BQ25730_HandleTypeDef *handle,
                                         BQ25730_AdcRawTypeDef *raw)
{
    uint16_t value;
    BQ25730_StatusTypeDef status;

    if (raw == NULL)
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    status = BQ25730_ReadRegister16(handle, BQ25730_REG_ADCVSYS_VBAT, &value);
    if (status != BQ25730_OK)
    {
        return status;
    }
    raw->vbat = (uint8_t)(value & 0x00FFU);
    raw->vsys = (uint8_t)((value >> 8U) & 0x00FFU);

    status = BQ25730_ReadRegister16(handle, BQ25730_REG_ADCIBAT, &value);
    if (status != BQ25730_OK)
    {
        return status;
    }
    raw->idchg = (uint8_t)(value & 0x007FU);
    raw->ichg = (uint8_t)((value >> 8U) & 0x007FU);

    status = BQ25730_ReadRegister16(handle, BQ25730_REG_ADCIIN_CMPIN, &value);
    if (status != BQ25730_OK)
    {
        return status;
    }
    raw->cmpin = (uint8_t)(value & 0x00FFU);
    raw->iin = (uint8_t)((value >> 8U) & 0x00FFU);

    return BQ25730_OK;
}

BQ25730_StatusTypeDef BQ25730_ReadAdcMeasurements(BQ25730_HandleTypeDef *handle,
                                                  BQ25730_AdcMeasurementsTypeDef *measurements)
{
    BQ25730_AdcRawTypeDef raw;
    BQ25730_StatusTypeDef status;

    if (measurements == NULL)
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    status = BQ25730_ReadAdcRaw(handle, &raw);
    if (status != BQ25730_OK)
    {
        return status;
    }

    measurements->battery_voltage_mv = BQ25730_ADC_VBAT_VSYS_OFFSET_MV +
                                       ((uint32_t)raw.vbat * BQ25730_ADC_VBAT_VSYS_STEP_MV);
    measurements->system_voltage_mv = BQ25730_ADC_VBAT_VSYS_OFFSET_MV +
                                      ((uint32_t)raw.vsys * BQ25730_ADC_VBAT_VSYS_STEP_MV);
    measurements->charge_current_ma = (uint32_t)raw.ichg * BQ25730_ADC_ICHG_STEP_MA;
    measurements->discharge_current_ma = (uint32_t)raw.idchg * BQ25730_ADC_IDCHG_STEP_MA;
    measurements->input_current_ma = (uint32_t)raw.iin * BQ25730_ADC_IIN_STEP_MA;
    measurements->cmpin_voltage_mv = (uint32_t)raw.cmpin * BQ25730_ADC_CMPIN_STEP_MV;

    return BQ25730_OK;
}

BQ25730_StatusTypeDef BQ25730_ConfigureComparatorLowOnLow(BQ25730_HandleTypeDef *handle)
{
    BQ25730_StatusTypeDef status;

    status = BQ25730_UpdateRegister16(handle,
                                      BQ25730_REG_CHARGE_OPTION1,
                                      BQ25730_CHARGE_OPTION1_CMP_REF |
                                      BQ25730_CHARGE_OPTION1_CMP_DEG_MASK |
                                      BQ25730_CHARGE_OPTION1_FORCE_CONV,
                                      BQ25730_CHARGE_OPTION1_CMP_POL |
                                      BQ25730_CHARGE_OPTION1_CMP_DEG_20MS);
    if (status != BQ25730_OK)
    {
        return status;
    }

    return BQ25730_UpdateRegister16(handle,
                                    BQ25730_REG_CHARGE_OPTION3,
                                    0U,
                                    BQ25730_CHARGE_OPTION3_CMP_EN);
}

BQ25730_StatusTypeDef BQ25730_ReadPins(BQ25730_HandleTypeDef *handle,
                                       uint32_t *pin_state)
{
    BQ25730_StatusTypeDef status;

    status = BQ25730_CheckHandle(handle);
    if (status != BQ25730_OK)
    {
        return status;
    }

    if (pin_state == NULL)
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    if (handle->bus.read_pins == NULL)
    {
        *pin_state = 0U;
        return BQ25730_OK;
    }

    return handle->bus.read_pins(handle->bus.context, pin_state);
}

BQ25730_StatusTypeDef BQ25730_ReadStatusSnapshot(BQ25730_HandleTypeDef *handle,
                                                 BQ25730_StatusSnapshotTypeDef *snapshot)
{
    BQ25730_StatusTypeDef status;

    if (snapshot == NULL)
    {
        return BQ25730_INVALID_ARGUMENT;
    }

    status = BQ25730_ReadChargerStatus(handle, &snapshot->charger_status);
    if (status != BQ25730_OK)
    {
        return status;
    }

    status = BQ25730_ReadProchotStatus(handle, &snapshot->prochot_status);
    if (status != BQ25730_OK)
    {
        return status;
    }

    return BQ25730_ReadPins(handle, &snapshot->pin_state);
}
