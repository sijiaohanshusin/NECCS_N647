/*
 * BQ25730 register driver.
 *
 * This layer is MCU-agnostic. Use BQ25730_BusTypeDef callbacks directly or
 * bind it to STM32 HAL through bq25730_hal.c.
 */

#ifndef BQ25730_H
#define BQ25730_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define BQ25730_I2C_ADDR_DEFAULT              0x6BU
#define BQ25730_MANUFACTURER_ID_EXPECTED      0x40U
#define BQ25730_DEVICE_ID_EXPECTED            0xD5U

#define BQ25730_REG_CHARGE_OPTION0            0x00U
#define BQ25730_REG_CHARGE_CURRENT            0x02U
#define BQ25730_REG_CHARGE_VOLTAGE            0x04U
#define BQ25730_REG_OTG_VOLTAGE               0x06U
#define BQ25730_REG_OTG_CURRENT               0x08U
#define BQ25730_REG_INPUT_VOLTAGE             0x0AU
#define BQ25730_REG_VSYS_MIN                  0x0CU
#define BQ25730_REG_IIN_HOST                  0x0EU
#define BQ25730_REG_CHARGER_STATUS            0x20U
#define BQ25730_REG_PROCHOT_STATUS            0x22U
#define BQ25730_REG_IIN_DPM                   0x24U
#define BQ25730_REG_ADCVBUS_PSYS              0x26U
#define BQ25730_REG_ADCIBAT                   0x28U
#define BQ25730_REG_ADCIIN_CMPIN              0x2AU
#define BQ25730_REG_ADCVSYS_VBAT              0x2CU
#define BQ25730_REG_MANUFACTURER_ID           0x2EU
#define BQ25730_REG_DEVICE_ID                 0x2FU
#define BQ25730_REG_CHARGE_OPTION1            0x30U
#define BQ25730_REG_CHARGE_OPTION2            0x32U
#define BQ25730_REG_CHARGE_OPTION3            0x34U
#define BQ25730_REG_PROCHOT_OPTION0           0x36U
#define BQ25730_REG_PROCHOT_OPTION1           0x38U
#define BQ25730_REG_ADC_OPTION                0x3AU
#define BQ25730_REG_CHARGE_OPTION4            0x3CU
#define BQ25730_REG_VMIN_ACTIVE_PROTECTION    0x3EU

#define BQ25730_PIN_STATE_PROCHOT             0x00000001UL
#define BQ25730_PIN_STATE_CHRG_OK             0x00000002UL
#define BQ25730_PIN_STATE_OTG_VAP             0x00000004UL
#define BQ25730_PIN_STATE_CMPOUT              0x00000008UL
#define BQ25730_PIN_STATE_PG                  0x00000010UL

#define BQ25730_CHARGER_STATUS_IN_OTG         0x0001U
#define BQ25730_CHARGER_STATUS_IN_PCHRG       0x0002U
#define BQ25730_CHARGER_STATUS_IN_FCHRG       0x0004U
#define BQ25730_CHARGER_STATUS_IN_IIN_DPM     0x0008U
#define BQ25730_CHARGER_STATUS_IN_VINDPM      0x0010U
#define BQ25730_CHARGER_STATUS_IN_VAP         0x0020U
#define BQ25730_CHARGER_STATUS_ICO_DONE       0x0040U
#define BQ25730_CHARGER_STATUS_STAT_AC        0x0080U
#define BQ25730_CHARGER_STATUS_FAULT_OTG_UVP  0x0100U
#define BQ25730_CHARGER_STATUS_FAULT_OTG_OVP  0x0200U
#define BQ25730_CHARGER_STATUS_FORCE_CONV_OFF 0x0400U
#define BQ25730_CHARGER_STATUS_FAULT_VSYS_UVP 0x0800U
#define BQ25730_CHARGER_STATUS_FAULT_SYSOVP   0x1000U
#define BQ25730_CHARGER_STATUS_FAULT_ACOC     0x2000U
#define BQ25730_CHARGER_STATUS_FAULT_BATOC    0x4000U
#define BQ25730_CHARGER_STATUS_FAULT_ACOV     0x8000U

#define BQ25730_ADC_CHANNEL_VBAT              0x0001U
#define BQ25730_ADC_CHANNEL_VSYS              0x0002U
#define BQ25730_ADC_CHANNEL_ICHG              0x0004U
#define BQ25730_ADC_CHANNEL_IDCHG             0x0008U
#define BQ25730_ADC_CHANNEL_IIN               0x0010U
#define BQ25730_ADC_CHANNEL_PSYS              0x0020U
#define BQ25730_ADC_CHANNEL_VBUS              0x0040U
#define BQ25730_ADC_CHANNEL_CMPIN             0x0080U
#define BQ25730_ADC_CHANNEL_ALL               0x00FFU

typedef enum
{
    BQ25730_OK = 0,
    BQ25730_ERROR = -1,
    BQ25730_INVALID_ARGUMENT = -2,
    BQ25730_IO_ERROR = -3,
    BQ25730_VERIFY_ERROR = -4
} BQ25730_StatusTypeDef;

typedef BQ25730_StatusTypeDef (*BQ25730_WriteRegFn)(void *context,
                                                    uint8_t address7,
                                                    uint8_t reg,
                                                    const uint8_t *data,
                                                    uint16_t length);
typedef BQ25730_StatusTypeDef (*BQ25730_ReadRegFn)(void *context,
                                                   uint8_t address7,
                                                   uint8_t reg,
                                                   uint8_t *data,
                                                   uint16_t length);
typedef void (*BQ25730_DelayMsFn)(void *context, uint32_t delay_ms);
typedef void (*BQ25730_SetOtgVapFn)(void *context, uint8_t enabled);
typedef BQ25730_StatusTypeDef (*BQ25730_ReadPinsFn)(void *context, uint32_t *pin_state);

typedef struct
{
    void *context;
    BQ25730_WriteRegFn write_reg;
    BQ25730_ReadRegFn read_reg;
    BQ25730_DelayMsFn delay_ms;
    BQ25730_SetOtgVapFn set_otg_vap;
    BQ25730_ReadPinsFn read_pins;
} BQ25730_BusTypeDef;

typedef struct
{
    uint8_t address7;
    BQ25730_BusTypeDef bus;
} BQ25730_HandleTypeDef;

typedef struct
{
    uint8_t manufacturer_id;
    uint8_t device_id;
} BQ25730_DeviceIdTypeDef;

typedef struct
{
    uint16_t charger_status;
    uint16_t prochot_status;
    uint32_t pin_state;
} BQ25730_StatusSnapshotTypeDef;

typedef struct
{
    uint8_t vbat;
    uint8_t vsys;
    uint8_t ichg;
    uint8_t idchg;
    uint8_t iin;
    uint8_t cmpin;
} BQ25730_AdcRawTypeDef;

typedef struct
{
    uint32_t battery_voltage_mv;
    uint32_t system_voltage_mv;
    uint32_t charge_current_ma;
    uint32_t discharge_current_ma;
    uint32_t input_current_ma;
    uint32_t cmpin_voltage_mv;
} BQ25730_AdcMeasurementsTypeDef;

BQ25730_StatusTypeDef BQ25730_Init(BQ25730_HandleTypeDef *handle,
                                   const BQ25730_BusTypeDef *bus,
                                   uint8_t address7);

BQ25730_StatusTypeDef BQ25730_Probe(BQ25730_HandleTypeDef *handle,
                                    BQ25730_DeviceIdTypeDef *device_id);

BQ25730_StatusTypeDef BQ25730_WriteRegister8(BQ25730_HandleTypeDef *handle,
                                             uint8_t reg,
                                             uint8_t value);

BQ25730_StatusTypeDef BQ25730_ReadRegister8(BQ25730_HandleTypeDef *handle,
                                            uint8_t reg,
                                            uint8_t *value);

BQ25730_StatusTypeDef BQ25730_WriteRegister16(BQ25730_HandleTypeDef *handle,
                                              uint8_t reg_lsb,
                                              uint16_t value);

BQ25730_StatusTypeDef BQ25730_ReadRegister16(BQ25730_HandleTypeDef *handle,
                                             uint8_t reg_lsb,
                                             uint16_t *value);

BQ25730_StatusTypeDef BQ25730_SetChargeCurrentMa(BQ25730_HandleTypeDef *handle,
                                                 uint32_t current_ma);

BQ25730_StatusTypeDef BQ25730_SetChargeVoltageMv(BQ25730_HandleTypeDef *handle,
                                                 uint32_t voltage_mv);

BQ25730_StatusTypeDef BQ25730_SetInputCurrentLimitMa(BQ25730_HandleTypeDef *handle,
                                                     uint32_t current_ma);

BQ25730_StatusTypeDef BQ25730_SetOtgVoltageMv(BQ25730_HandleTypeDef *handle,
                                              uint32_t voltage_mv);

BQ25730_StatusTypeDef BQ25730_SetOtgCurrentMa(BQ25730_HandleTypeDef *handle,
                                             uint32_t current_ma);

BQ25730_StatusTypeDef BQ25730_SetOtgEnabled(BQ25730_HandleTypeDef *handle,
                                            uint8_t enabled);

BQ25730_StatusTypeDef BQ25730_ConfigureAdc(BQ25730_HandleTypeDef *handle,
                                           uint16_t adc_channels,
                                           uint8_t continuous);

BQ25730_StatusTypeDef BQ25730_ReadAdcRaw(BQ25730_HandleTypeDef *handle,
                                         BQ25730_AdcRawTypeDef *raw);

BQ25730_StatusTypeDef BQ25730_ReadAdcMeasurements(BQ25730_HandleTypeDef *handle,
                                                  BQ25730_AdcMeasurementsTypeDef *measurements);

BQ25730_StatusTypeDef BQ25730_ConfigureComparatorLowOnLow(BQ25730_HandleTypeDef *handle);

BQ25730_StatusTypeDef BQ25730_ReadChargerStatus(BQ25730_HandleTypeDef *handle,
                                                uint16_t *charger_status);

BQ25730_StatusTypeDef BQ25730_ReadProchotStatus(BQ25730_HandleTypeDef *handle,
                                                uint16_t *prochot_status);

BQ25730_StatusTypeDef BQ25730_ReadPins(BQ25730_HandleTypeDef *handle,
                                       uint32_t *pin_state);

BQ25730_StatusTypeDef BQ25730_ReadStatusSnapshot(BQ25730_HandleTypeDef *handle,
                                                 BQ25730_StatusSnapshotTypeDef *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* BQ25730_H */
