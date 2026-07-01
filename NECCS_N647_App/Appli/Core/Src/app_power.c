#include "app_power.h"

#include "app_debug_config.h"
#include "main.h"
#include "BQ25730/bq25730_hal.h"

#include <string.h>

#define APP_POWER_ADC_CHANNELS (BQ25730_ADC_CHANNEL_VBAT | \
                                BQ25730_ADC_CHANNEL_VSYS | \
                                BQ25730_ADC_CHANNEL_ICHG | \
                                BQ25730_ADC_CHANNEL_IDCHG | \
                                BQ25730_ADC_CHANNEL_IIN | \
                                BQ25730_ADC_CHANNEL_PSYS | \
                                BQ25730_ADC_CHANNEL_VBUS | \
                                BQ25730_ADC_CHANNEL_CMPIN)

#define APP_POWER_CHARGE_STATUS_MASK (BQ25730_CHARGER_STATUS_IN_PCHRG | \
                                      BQ25730_CHARGER_STATUS_IN_FCHRG)

#define APP_POWER_CHARGER_FAULT_MASK (BQ25730_CHARGER_STATUS_FAULT_OTG_UVP | \
                                      BQ25730_CHARGER_STATUS_FAULT_OTG_OVP | \
                                      BQ25730_CHARGER_STATUS_FAULT_VSYS_UVP | \
                                      BQ25730_CHARGER_STATUS_FAULT_SYSOVP | \
                                      BQ25730_CHARGER_STATUS_FAULT_ACOC | \
                                      BQ25730_CHARGER_STATUS_FAULT_BATOC | \
                                      BQ25730_CHARGER_STATUS_FAULT_ACOV)

#define APP_POWER_PIN_FLAG_MASK (APP_POWER_FLAG_PROCHOT_HIGH | \
                                 APP_POWER_FLAG_CHRG_OK_HIGH | \
                                 APP_POWER_FLAG_OTG_VAP_HIGH | \
                                 APP_POWER_FLAG_CMPOUT_HIGH | \
                                 APP_POWER_FLAG_PG_HIGH)

typedef struct
{
  uint16_t millivolts;
  uint8_t percent;
} AppPowerSocPoint_t;

volatile int32_t g_app_bq25730_init_status = BQ25730_ERROR;
volatile int32_t g_app_bq25730_probe_status = BQ25730_ERROR;
volatile int32_t g_app_bq25730_last_i2c_status = BQ25730_ERROR;
volatile int32_t g_app_bq25730_adc_status = BQ25730_ERROR;
volatile int32_t g_app_bq25730_pin_read_status = BQ25730_ERROR;
volatile uint32_t g_app_bq25730_manufacturer_id = 0U;
volatile uint32_t g_app_bq25730_device_id = 0U;
volatile uint32_t g_app_bq25730_charger_status = 0U;
volatile uint32_t g_app_bq25730_prochot_status = 0U;
volatile uint32_t g_app_bq25730_pin_state = 0U;
volatile uint32_t g_app_bq25730_refresh_count = 0U;
volatile uint32_t g_app_bq25730_charge_option0 = 0U;
volatile uint32_t g_app_bq25730_charge_option1 = 0U;
volatile uint32_t g_app_bq25730_charge_option2 = 0U;
volatile uint32_t g_app_bq25730_charge_option3 = 0U;
volatile uint32_t g_app_bq25730_adc_option = 0U;
volatile uint32_t g_app_bq25730_charge_voltage_reg = 0U;
volatile uint32_t g_app_bq25730_charge_current_reg = 0U;
volatile uint32_t g_app_bq25730_input_current_reg = 0U;
volatile uint32_t g_app_bq25730_vsys_min_reg = 0U;
volatile uint32_t g_app_bq25730_otg_voltage_reg = 0U;
volatile uint32_t g_app_bq25730_otg_current_reg = 0U;
volatile uint32_t g_app_bq25730_adc_raw_vbat = 0U;
volatile uint32_t g_app_bq25730_adc_raw_vsys = 0U;
volatile uint32_t g_app_bq25730_adc_raw_psys = 0U;
volatile uint32_t g_app_bq25730_adc_raw_vbus = 0U;
volatile uint32_t g_app_bq25730_adc_raw_ichg = 0U;
volatile uint32_t g_app_bq25730_adc_raw_idchg = 0U;
volatile uint32_t g_app_bq25730_adc_raw_iin = 0U;
volatile uint32_t g_app_bq25730_adc_raw_cmpin = 0U;
volatile int32_t g_app_bq25730_low_power_status = BQ25730_ERROR;
volatile uint32_t g_app_bq25730_low_power_command_count = 0U;
volatile uint32_t g_app_bq25730_low_power_enabled = 0U;
volatile uint32_t g_app_bq25730_low_power_target = 0U;
volatile uint32_t g_app_bq25730_low_power_request_pending = 0U;

volatile uint32_t g_app_power_flags = 0U;
volatile uint32_t g_app_power_state = APP_POWER_STATE_UNKNOWN;
volatile uint32_t g_app_power_battery_mv = 0U;
volatile uint32_t g_app_power_system_mv = 0U;
volatile uint32_t g_app_power_input_voltage_mv = 0U;
volatile uint32_t g_app_power_psys_mv = 0U;
volatile uint32_t g_app_power_battery_percent = 0U;
volatile uint32_t g_app_power_cmpin_mv = 0U;
volatile int32_t g_app_power_battery_current_ma = 0;
volatile int32_t g_app_power_remaining_mah_x1000 = -1;

static BQ25730_HandleTypeDef g_app_bq25730;
static BQ25730_BusTypeDef g_app_bq25730_bus;
static BQ25730_HAL_BusContextTypeDef g_app_bq25730_hal_context =
{
  .hi2c = &hi2c2,
  .prochot_port = BQ25730_PROCHOT_GPIO_Port,
  .prochot_pin = BQ25730_PROCHOT_Pin,
  .chrg_ok_port = BQ25730_CHRG_OK_GPIO_Port,
  .chrg_ok_pin = BQ25730_CHRG_OK_Pin,
  .otg_vap_port = BQ25730_OTG_VAP_GPIO_Port,
  .otg_vap_pin = BQ25730_OTG_VAP_Pin,
  .cmpout_port = BQ25730_CMPOUT_GPIO_Port,
  .cmpout_pin = BQ25730_CMPOUT_Pin,
  .pg_port = BQ25730_PG_GPIO_Port,
  .pg_pin = BQ25730_PG_Pin,
  .timeout_ms = 100U,
};

static AppPowerSnapshot_t g_app_power_snapshot;
static uint32_t g_app_power_undervoltage_ms;
static uint8_t g_app_power_remaining_initialized;

static const AppPowerSocPoint_t g_app_power_soc_table[] =
{
  {9000U, 0U},
  {9600U, 5U},
  {9900U, 10U},
  {10500U, 20U},
  {10800U, 30U},
  {11100U, 40U},
  {11400U, 55U},
  {11700U, 70U},
  {12000U, 85U},
  {12300U, 95U},
  {12600U, 100U},
};

static void AppPower_UpdateDebugGlobals(void)
{
  g_app_bq25730_init_status = g_app_power_snapshot.init_status;
  g_app_bq25730_probe_status = g_app_power_snapshot.probe_status;
  g_app_bq25730_last_i2c_status = g_app_power_snapshot.last_i2c_status;
  g_app_bq25730_adc_status = g_app_power_snapshot.adc_status;
  g_app_bq25730_pin_read_status = g_app_power_snapshot.pin_read_status;
  g_app_bq25730_manufacturer_id = g_app_power_snapshot.manufacturer_id;
  g_app_bq25730_device_id = g_app_power_snapshot.device_id;
  g_app_bq25730_charger_status = g_app_power_snapshot.charger_status;
  g_app_bq25730_prochot_status = g_app_power_snapshot.prochot_status;
  g_app_bq25730_pin_state = g_app_power_snapshot.pin_state;
  g_app_bq25730_refresh_count = g_app_power_snapshot.update_count;
  g_app_bq25730_charge_option0 = g_app_power_snapshot.charge_option0;
  g_app_bq25730_charge_option1 = g_app_power_snapshot.charge_option1;
  g_app_bq25730_charge_option2 = g_app_power_snapshot.charge_option2;
  g_app_bq25730_charge_option3 = g_app_power_snapshot.charge_option3;
  g_app_bq25730_adc_option = g_app_power_snapshot.adc_option;
  g_app_bq25730_charge_voltage_reg = g_app_power_snapshot.charge_voltage_reg;
  g_app_bq25730_charge_current_reg = g_app_power_snapshot.charge_current_reg;
  g_app_bq25730_input_current_reg = g_app_power_snapshot.input_current_reg;
  g_app_bq25730_vsys_min_reg = g_app_power_snapshot.vsys_min_reg;
  g_app_bq25730_otg_voltage_reg = g_app_power_snapshot.otg_voltage_reg;
  g_app_bq25730_otg_current_reg = g_app_power_snapshot.otg_current_reg;
  g_app_bq25730_adc_raw_vbat = g_app_power_snapshot.adc_raw_vbat;
  g_app_bq25730_adc_raw_vsys = g_app_power_snapshot.adc_raw_vsys;
  g_app_bq25730_adc_raw_psys = g_app_power_snapshot.adc_raw_psys;
  g_app_bq25730_adc_raw_vbus = g_app_power_snapshot.adc_raw_vbus;
  g_app_bq25730_adc_raw_ichg = g_app_power_snapshot.adc_raw_ichg;
  g_app_bq25730_adc_raw_idchg = g_app_power_snapshot.adc_raw_idchg;
  g_app_bq25730_adc_raw_iin = g_app_power_snapshot.adc_raw_iin;
  g_app_bq25730_adc_raw_cmpin = g_app_power_snapshot.adc_raw_cmpin;
  g_app_bq25730_low_power_status = g_app_power_snapshot.low_power_status;
  g_app_bq25730_low_power_command_count = g_app_power_snapshot.low_power_command_count;
  g_app_bq25730_low_power_enabled = g_app_power_snapshot.low_power_enabled;
  g_app_bq25730_low_power_target = g_app_power_snapshot.low_power_target;
  g_app_bq25730_low_power_request_pending = g_app_power_snapshot.low_power_request_pending;

  g_app_power_flags = g_app_power_snapshot.flags;
  g_app_power_state = g_app_power_snapshot.state;
  g_app_power_battery_mv = g_app_power_snapshot.battery_mv;
  g_app_power_system_mv = g_app_power_snapshot.system_mv;
  g_app_power_input_voltage_mv = g_app_power_snapshot.input_voltage_mv;
  g_app_power_psys_mv = g_app_power_snapshot.psys_mv;
  g_app_power_battery_percent = g_app_power_snapshot.battery_percent;
  g_app_power_cmpin_mv = g_app_power_snapshot.cmpin_mv;
  g_app_power_battery_current_ma = g_app_power_snapshot.battery_current_ma;
  g_app_power_remaining_mah_x1000 = g_app_power_snapshot.remaining_mah_x1000;
}

static void AppPower_ResetSnapshot(void)
{
  (void)memset(&g_app_power_snapshot, 0, sizeof(g_app_power_snapshot));
  g_app_power_snapshot.init_status = BQ25730_ERROR;
  g_app_power_snapshot.probe_status = BQ25730_ERROR;
  g_app_power_snapshot.last_i2c_status = BQ25730_ERROR;
  g_app_power_snapshot.adc_status = BQ25730_ERROR;
  g_app_power_snapshot.pin_read_status = BQ25730_ERROR;
  g_app_power_snapshot.low_power_status = BQ25730_ERROR;
  g_app_power_snapshot.remaining_mah_x1000 = -1;
  g_app_power_snapshot.state = APP_POWER_STATE_UNKNOWN;
  g_app_power_undervoltage_ms = 0U;
  g_app_power_remaining_initialized = 0U;
  AppPower_UpdateDebugGlobals();
}

static void AppPower_RecordPins(uint32_t pin_state)
{
  g_app_power_snapshot.pin_state = pin_state;
  g_app_power_snapshot.flags &= ~APP_POWER_PIN_FLAG_MASK;

  if ((pin_state & BQ25730_PIN_STATE_PROCHOT) != 0U)
  {
    g_app_power_snapshot.flags |= APP_POWER_FLAG_PROCHOT_HIGH;
  }
  if ((pin_state & BQ25730_PIN_STATE_CHRG_OK) != 0U)
  {
    g_app_power_snapshot.flags |= APP_POWER_FLAG_CHRG_OK_HIGH;
  }
  if ((pin_state & BQ25730_PIN_STATE_OTG_VAP) != 0U)
  {
    g_app_power_snapshot.flags |= APP_POWER_FLAG_OTG_VAP_HIGH;
  }
  if ((pin_state & BQ25730_PIN_STATE_CMPOUT) != 0U)
  {
    g_app_power_snapshot.flags |= APP_POWER_FLAG_CMPOUT_HIGH;
  }
  if ((pin_state & BQ25730_PIN_STATE_PG) != 0U)
  {
    g_app_power_snapshot.flags |= APP_POWER_FLAG_PG_HIGH;
  }
}

static uint8_t AppPower_EstimatePercentFromVoltage(uint32_t battery_mv)
{
  uint32_t i;

  if (battery_mv <= g_app_power_soc_table[0].millivolts)
  {
    return g_app_power_soc_table[0].percent;
  }

  for (i = 1U; i < (sizeof(g_app_power_soc_table) / sizeof(g_app_power_soc_table[0])); i++)
  {
    const AppPowerSocPoint_t *low = &g_app_power_soc_table[i - 1U];
    const AppPowerSocPoint_t *high = &g_app_power_soc_table[i];

    if (battery_mv <= high->millivolts)
    {
      uint32_t voltage_span = (uint32_t)high->millivolts - low->millivolts;
      uint32_t percent_span = (uint32_t)high->percent - low->percent;
      uint32_t voltage_delta = battery_mv - low->millivolts;
      uint32_t percent = low->percent;

      if (voltage_span != 0U)
      {
        percent += (voltage_delta * percent_span) / voltage_span;
      }

      return (uint8_t)percent;
    }
  }

  return g_app_power_soc_table[(sizeof(g_app_power_soc_table) / sizeof(g_app_power_soc_table[0])) - 1U].percent;
}

static int32_t AppPower_ClampRemainingMahX1000(int64_t value)
{
  const int64_t max_value = (int64_t)APP_POWER_PACK_CAPACITY_MAH * 1000;

  if (value < 0)
  {
    return 0;
  }
  if (value > max_value)
  {
    return (int32_t)max_value;
  }

  return (int32_t)value;
}

static void AppPower_UpdateState(void)
{
  g_app_power_snapshot.flags &= ~APP_POWER_FLAG_CHARGER_FAULT;

  if ((g_app_power_snapshot.charger_status & APP_POWER_CHARGER_FAULT_MASK) != 0U)
  {
    g_app_power_snapshot.flags |= APP_POWER_FLAG_CHARGER_FAULT;
    g_app_power_snapshot.state = APP_POWER_STATE_FAULT;
  }
  else if ((g_app_power_snapshot.flags & APP_POWER_FLAG_UNDERVOLTAGE_CONFIRMED) != 0U)
  {
    g_app_power_snapshot.state = APP_POWER_STATE_UNDERVOLTAGE;
  }
  else if ((g_app_power_snapshot.charger_status & BQ25730_CHARGER_STATUS_IN_OTG) != 0U)
  {
    g_app_power_snapshot.state = APP_POWER_STATE_OTG;
  }
  else if ((g_app_power_snapshot.charger_status & APP_POWER_CHARGE_STATUS_MASK) != 0U)
  {
    g_app_power_snapshot.state = APP_POWER_STATE_CHARGING;
  }
  else if (g_app_power_snapshot.battery_current_ma < 0)
  {
    g_app_power_snapshot.state = APP_POWER_STATE_DISCHARGING;
  }
  else if ((g_app_power_snapshot.flags & APP_POWER_FLAG_BQ_PRESENT) != 0U)
  {
    g_app_power_snapshot.state = APP_POWER_STATE_IDLE;
  }
  else
  {
    g_app_power_snapshot.state = APP_POWER_STATE_UNKNOWN;
  }
}

static void AppPower_ProcessUndervoltage(uint32_t elapsed_ms)
{
  uint8_t undervoltage_raw = 0U;

  g_app_power_snapshot.flags &= ~(APP_POWER_FLAG_UNDERVOLTAGE_RAW |
                                  APP_POWER_FLAG_UNDERVOLTAGE_CONFIRMED);

  if ((g_app_power_snapshot.flags & APP_POWER_FLAG_BQ_PRESENT) != 0U)
  {
    undervoltage_raw = ((g_app_power_snapshot.pin_state & BQ25730_PIN_STATE_CMPOUT) == 0U) ? 1U : 0U;
  }

  if (((g_app_power_snapshot.flags & APP_POWER_FLAG_ADC_VALID) != 0U) &&
      (g_app_power_snapshot.battery_mv <= APP_POWER_3S_UNDERVOLTAGE_MV))
  {
    undervoltage_raw = 1U;
  }

  if (undervoltage_raw != 0U)
  {
    g_app_power_snapshot.flags |= APP_POWER_FLAG_UNDERVOLTAGE_RAW;
    if (g_app_power_undervoltage_ms < APP_POWER_UNDERVOLTAGE_DEBOUNCE_MS)
    {
      g_app_power_undervoltage_ms += elapsed_ms;
    }
    if (g_app_power_undervoltage_ms >= APP_POWER_UNDERVOLTAGE_DEBOUNCE_MS)
    {
      g_app_power_snapshot.flags |= APP_POWER_FLAG_UNDERVOLTAGE_CONFIRMED;
    }
  }
  else
  {
    g_app_power_undervoltage_ms = 0U;
  }
}

static void AppPower_UpdateBatteryCurrent(void)
{
  if (((g_app_power_snapshot.charger_status & APP_POWER_CHARGE_STATUS_MASK) != 0U) &&
      (g_app_power_snapshot.charge_current_ma != 0U))
  {
    g_app_power_snapshot.battery_current_ma = (int32_t)g_app_power_snapshot.charge_current_ma;
  }
  else if (g_app_power_snapshot.discharge_current_ma != 0U)
  {
    g_app_power_snapshot.battery_current_ma = -(int32_t)g_app_power_snapshot.discharge_current_ma;
  }
  else
  {
    g_app_power_snapshot.battery_current_ma = 0;
  }
}

static void AppPower_UpdateRemainingCapacity(uint32_t elapsed_ms)
{
  const int32_t capacity_x1000 = (int32_t)APP_POWER_PACK_CAPACITY_MAH * 1000;

  if ((g_app_power_snapshot.flags & APP_POWER_FLAG_ADC_VALID) == 0U)
  {
    return;
  }

  if (g_app_power_remaining_initialized == 0U)
  {
    uint8_t voltage_percent = AppPower_EstimatePercentFromVoltage(g_app_power_snapshot.battery_mv);
    g_app_power_snapshot.remaining_mah_x1000 =
      (int32_t)(((uint32_t)APP_POWER_PACK_CAPACITY_MAH * 1000U * voltage_percent) / 100U);
    g_app_power_remaining_initialized = 1U;
  }

  if (elapsed_ms != 0U)
  {
    int64_t remaining = g_app_power_snapshot.remaining_mah_x1000;
    int64_t delta = ((int64_t)g_app_power_snapshot.battery_current_ma * (int64_t)elapsed_ms) / 3600;

    remaining += delta;
    g_app_power_snapshot.remaining_mah_x1000 = AppPower_ClampRemainingMahX1000(remaining);
  }

  if (capacity_x1000 > 0)
  {
    g_app_power_snapshot.battery_percent =
      (uint8_t)(((g_app_power_snapshot.remaining_mah_x1000 * 100) + (capacity_x1000 / 2)) / capacity_x1000);
  }
}

static void AppPower_ClearAdcSnapshot(void)
{
  g_app_power_snapshot.flags &= ~APP_POWER_FLAG_ADC_VALID;
  g_app_power_snapshot.battery_mv = 0U;
  g_app_power_snapshot.system_mv = 0U;
  g_app_power_snapshot.input_voltage_mv = 0U;
  g_app_power_snapshot.psys_mv = 0U;
  g_app_power_snapshot.charge_current_ma = 0U;
  g_app_power_snapshot.discharge_current_ma = 0U;
  g_app_power_snapshot.input_current_ma = 0U;
  g_app_power_snapshot.cmpin_mv = 0U;
  g_app_power_snapshot.battery_current_ma = 0;
  g_app_power_snapshot.adc_raw_vbat = 0U;
  g_app_power_snapshot.adc_raw_vsys = 0U;
  g_app_power_snapshot.adc_raw_psys = 0U;
  g_app_power_snapshot.adc_raw_vbus = 0U;
  g_app_power_snapshot.adc_raw_ichg = 0U;
  g_app_power_snapshot.adc_raw_idchg = 0U;
  g_app_power_snapshot.adc_raw_iin = 0U;
  g_app_power_snapshot.adc_raw_cmpin = 0U;
}

static void AppPower_RecordAdc(const BQ25730_AdcMeasurementsTypeDef *measurements,
                               uint32_t elapsed_ms)
{
  g_app_power_snapshot.flags |= APP_POWER_FLAG_ADC_VALID;
  g_app_power_snapshot.battery_mv = measurements->battery_voltage_mv;
  g_app_power_snapshot.system_mv = measurements->system_voltage_mv;
  g_app_power_snapshot.input_voltage_mv = measurements->input_voltage_mv;
  g_app_power_snapshot.psys_mv = measurements->psys_voltage_mv;
  g_app_power_snapshot.charge_current_ma = measurements->charge_current_ma;
  g_app_power_snapshot.discharge_current_ma = measurements->discharge_current_ma;
  g_app_power_snapshot.input_current_ma = measurements->input_current_ma;
  g_app_power_snapshot.cmpin_mv = measurements->cmpin_voltage_mv;
  g_app_power_snapshot.adc_raw_vbat = measurements->raw_vbat;
  g_app_power_snapshot.adc_raw_vsys = measurements->raw_vsys;
  g_app_power_snapshot.adc_raw_psys = measurements->raw_psys;
  g_app_power_snapshot.adc_raw_vbus = measurements->raw_vbus;
  g_app_power_snapshot.adc_raw_ichg = measurements->raw_ichg;
  g_app_power_snapshot.adc_raw_idchg = measurements->raw_idchg;
  g_app_power_snapshot.adc_raw_iin = measurements->raw_iin;
  g_app_power_snapshot.adc_raw_cmpin = measurements->raw_cmpin;

  AppPower_UpdateBatteryCurrent();
  AppPower_UpdateRemainingCapacity(elapsed_ms);
}

static void AppPower_ReadPins(void)
{
  uint32_t pin_state = 0U;
  BQ25730_StatusTypeDef status;

  if (g_app_power_snapshot.init_status != BQ25730_OK)
  {
    return;
  }

  status = BQ25730_ReadPins(&g_app_bq25730, &pin_state);
  g_app_power_snapshot.pin_read_status = (int32_t)status;
  if (status == BQ25730_OK)
  {
    AppPower_RecordPins(pin_state);
  }
}

static void AppPower_MarkBqOffline(BQ25730_StatusTypeDef status)
{
  g_app_power_snapshot.flags &= ~(APP_POWER_FLAG_BQ_PRESENT | APP_POWER_FLAG_ADC_VALID);
  g_app_power_snapshot.probe_status = (int32_t)status;
  g_app_power_snapshot.last_i2c_status = (int32_t)status;
}

static BQ25730_StatusTypeDef AppPower_ProbeDevice(void)
{
  BQ25730_DeviceIdTypeDef device_id = {0U, 0U};
  BQ25730_StatusTypeDef status;

  if (g_app_power_snapshot.init_status != BQ25730_OK)
  {
    return BQ25730_ERROR;
  }

  status = BQ25730_Probe(&g_app_bq25730, &device_id);
  g_app_power_snapshot.probe_status = (int32_t)status;
  g_app_power_snapshot.last_i2c_status = (int32_t)status;
  g_app_power_snapshot.manufacturer_id = device_id.manufacturer_id;
  g_app_power_snapshot.device_id = device_id.device_id;

  if (status == BQ25730_OK)
  {
    g_app_power_snapshot.flags |= APP_POWER_FLAG_BQ_PRESENT;
  }
  else
  {
    AppPower_MarkBqOffline(status);
  }

  return status;
}

static BQ25730_StatusTypeDef AppPower_ReadDebugRegister16(uint8_t reg,
                                                          uint16_t *value)
{
  BQ25730_StatusTypeDef status;

  status = BQ25730_ReadRegister16(&g_app_bq25730, reg, value);
  g_app_power_snapshot.last_i2c_status = (int32_t)status;
  if (status != BQ25730_OK)
  {
    AppPower_MarkBqOffline(status);
  }

  return status;
}

static void AppPower_ReadDebugRegisters(void)
{
  uint16_t value = 0U;

  if (g_app_power_snapshot.probe_status != BQ25730_OK)
  {
    return;
  }

  if (AppPower_ReadDebugRegister16(BQ25730_REG_CHARGE_OPTION0, &value) != BQ25730_OK)
  {
    return;
  }
  g_app_power_snapshot.charge_option0 = value;
  g_app_power_snapshot.low_power_enabled =
    ((value & BQ25730_CHARGE_OPTION0_EN_LWPWR) != 0U) ? 1U : 0U;

  if (AppPower_ReadDebugRegister16(BQ25730_REG_CHARGE_CURRENT, &g_app_power_snapshot.charge_current_reg) != BQ25730_OK)
  {
    return;
  }
  if (AppPower_ReadDebugRegister16(BQ25730_REG_CHARGE_VOLTAGE, &g_app_power_snapshot.charge_voltage_reg) != BQ25730_OK)
  {
    return;
  }
  if (AppPower_ReadDebugRegister16(BQ25730_REG_OTG_VOLTAGE, &g_app_power_snapshot.otg_voltage_reg) != BQ25730_OK)
  {
    return;
  }
  if (AppPower_ReadDebugRegister16(BQ25730_REG_OTG_CURRENT, &g_app_power_snapshot.otg_current_reg) != BQ25730_OK)
  {
    return;
  }
  if (AppPower_ReadDebugRegister16(BQ25730_REG_VSYS_MIN, &g_app_power_snapshot.vsys_min_reg) != BQ25730_OK)
  {
    return;
  }
  if (AppPower_ReadDebugRegister16(BQ25730_REG_IIN_HOST, &g_app_power_snapshot.input_current_reg) != BQ25730_OK)
  {
    return;
  }
  if (AppPower_ReadDebugRegister16(BQ25730_REG_CHARGE_OPTION1, &g_app_power_snapshot.charge_option1) != BQ25730_OK)
  {
    return;
  }
  if (AppPower_ReadDebugRegister16(BQ25730_REG_CHARGE_OPTION2, &g_app_power_snapshot.charge_option2) != BQ25730_OK)
  {
    return;
  }
  if (AppPower_ReadDebugRegister16(BQ25730_REG_CHARGE_OPTION3, &g_app_power_snapshot.charge_option3) != BQ25730_OK)
  {
    return;
  }
  (void)AppPower_ReadDebugRegister16(BQ25730_REG_ADC_OPTION, &g_app_power_snapshot.adc_option);
}

static void AppPower_DelayMs(uint32_t delay_ms)
{
  if (g_app_bq25730.bus.delay_ms != NULL)
  {
    g_app_bq25730.bus.delay_ms(g_app_bq25730.bus.context, delay_ms);
  }
}

static BQ25730_StatusTypeDef AppPower_RestartAdc(void)
{
  BQ25730_StatusTypeDef status;

  status = BQ25730_ConfigureAdc(&g_app_bq25730, APP_POWER_ADC_CHANNELS, 1U);
  g_app_power_snapshot.adc_status = (int32_t)status;
  g_app_power_snapshot.last_i2c_status = (int32_t)status;
  if (status != BQ25730_OK)
  {
    AppPower_ClearAdcSnapshot();
  }

  return status;
}

static BQ25730_StatusTypeDef AppPower_ConfigureDetectedDevice(void)
{
  BQ25730_StatusTypeDef status;

  status = BQ25730_SetOtgEnabled(&g_app_bq25730, 0U);
  g_app_power_snapshot.last_i2c_status = (int32_t)status;
  if (status == BQ25730_OK)
  {
    status = BQ25730_SetChargeVoltageMv(&g_app_bq25730, APP_POWER_3S_CHARGE_VOLTAGE_MV);
    g_app_power_snapshot.last_i2c_status = (int32_t)status;
  }
  if (status == BQ25730_OK)
  {
    status = BQ25730_SetChargeCurrentMa(&g_app_bq25730, APP_POWER_DEFAULT_CHARGE_CURRENT_MA);
    g_app_power_snapshot.last_i2c_status = (int32_t)status;
  }
  if (status == BQ25730_OK)
  {
    status = BQ25730_ConfigureComparatorLowOnLow(&g_app_bq25730);
    g_app_power_snapshot.last_i2c_status = (int32_t)status;
  }
#if APP_TEMP_BQ_DEBUG_MODE
  if (status == BQ25730_OK)
  {
    status = BQ25730_SetLowPowerMode(&g_app_bq25730, 0U);
    g_app_power_snapshot.low_power_status = (int32_t)status;
    g_app_power_snapshot.last_i2c_status = (int32_t)status;
    g_app_power_snapshot.low_power_target = 0U;
    if (status == BQ25730_OK)
    {
      g_app_power_snapshot.low_power_enabled = 0U;
      AppPower_DelayMs(10U);
    }
  }
#endif
  if (status == BQ25730_OK)
  {
    (void)AppPower_RestartAdc();
  }

  if (status != BQ25730_OK)
  {
    AppPower_MarkBqOffline(status);
  }

  return status;
}

static void AppPower_ProcessLowPowerRequest(void)
{
  BQ25730_StatusTypeDef status;

  if ((g_app_power_snapshot.low_power_request_pending == 0U) ||
      (g_app_power_snapshot.probe_status != BQ25730_OK))
  {
    return;
  }

  g_app_power_snapshot.low_power_request_pending = 0U;
  status = BQ25730_SetLowPowerMode(&g_app_bq25730,
                                   g_app_power_snapshot.low_power_target);
  g_app_power_snapshot.low_power_status = (int32_t)status;
  g_app_power_snapshot.last_i2c_status = (int32_t)status;
  g_app_power_snapshot.low_power_command_count++;

  if (status == BQ25730_OK)
  {
    g_app_power_snapshot.low_power_enabled =
      (g_app_power_snapshot.low_power_target == 0U) ? 0U : 1U;
    if (g_app_power_snapshot.low_power_target == 0U)
    {
      AppPower_DelayMs(10U);
      (void)AppPower_RestartAdc();
    }
    else
    {
      AppPower_ClearAdcSnapshot();
    }
  }
  else
  {
    AppPower_MarkBqOffline(status);
  }
}

static void AppPower_ReadLiveStatus(uint32_t elapsed_ms, uint8_t read_prochot)
{
  BQ25730_AdcMeasurementsTypeDef measurements;
  uint16_t charger_status = 0U;
  uint16_t prochot_status = 0U;
  BQ25730_StatusTypeDef status;

  if (g_app_power_snapshot.probe_status != BQ25730_OK)
  {
    return;
  }

  status = BQ25730_ReadChargerStatus(&g_app_bq25730, &charger_status);
  g_app_power_snapshot.last_i2c_status = (int32_t)status;
  if (status != BQ25730_OK)
  {
    AppPower_MarkBqOffline(status);
    return;
  }
  g_app_power_snapshot.charger_status = charger_status;

  if (read_prochot != 0U)
  {
    status = BQ25730_ReadProchotStatus(&g_app_bq25730, &prochot_status);
    g_app_power_snapshot.last_i2c_status = (int32_t)status;
    if (status != BQ25730_OK)
    {
      AppPower_MarkBqOffline(status);
      return;
    }
    g_app_power_snapshot.prochot_status = prochot_status;
  }

  if (g_app_power_snapshot.low_power_enabled != 0U)
  {
    g_app_power_snapshot.adc_status = BQ25730_ERROR;
    AppPower_ClearAdcSnapshot();
  }
  else
  {
    status = BQ25730_ReadAdcMeasurements(&g_app_bq25730, &measurements);
    g_app_power_snapshot.adc_status = (int32_t)status;
    if (status == BQ25730_OK)
    {
      AppPower_RecordAdc(&measurements, elapsed_ms);
    }
    else
    {
      AppPower_ClearAdcSnapshot();
    }
  }

  AppPower_ReadDebugRegisters();
}

void AppPower_Init(void)
{
  BQ25730_StatusTypeDef status;

  AppPower_ResetSnapshot();

  BQ25730_HAL_SetOtgVap(&g_app_bq25730_hal_context, 0U);
  BQ25730_HAL_BusInit(&g_app_bq25730_bus, &g_app_bq25730_hal_context);

  status = BQ25730_Init(&g_app_bq25730,
                        &g_app_bq25730_bus,
                        BQ25730_I2C_ADDR_DEFAULT);
  g_app_power_snapshot.init_status = (int32_t)status;
  if (status != BQ25730_OK)
  {
    AppPower_UpdateDebugGlobals();
    return;
  }

  AppPower_ReadPins();

  status = AppPower_ProbeDevice();
  if (status == BQ25730_OK)
  {
    status = AppPower_ConfigureDetectedDevice();
    if (status == BQ25730_OK)
    {
      AppPower_ReadLiveStatus(0U, 1U);
    }
  }

  AppPower_ProcessUndervoltage(0U);
  AppPower_UpdateState();
  AppPower_UpdateDebugGlobals();
}

void AppPower_Poll(uint32_t elapsed_ms)
{
  BQ25730_StatusTypeDef status;

  AppPower_ReadPins();

  if ((g_app_power_snapshot.init_status == BQ25730_OK) &&
      (g_app_power_snapshot.probe_status != BQ25730_OK))
  {
    status = AppPower_ProbeDevice();
    if (status == BQ25730_OK)
    {
      (void)AppPower_ConfigureDetectedDevice();
      AppPower_ReadLiveStatus(0U, 1U);
    }
  }

  AppPower_ProcessLowPowerRequest();
  AppPower_ReadLiveStatus(elapsed_ms, 0U);
  AppPower_ProcessUndervoltage(elapsed_ms);
  AppPower_UpdateState();
  g_app_power_snapshot.update_count++;
  AppPower_UpdateDebugGlobals();
}

void AppPower_GetSnapshot(AppPowerSnapshot_t *snapshot)
{
  if (snapshot != NULL)
  {
    *snapshot = g_app_power_snapshot;
  }
}

int32_t AppPower_SetCharging(uint8_t enabled, uint32_t current_ma)
{
  BQ25730_StatusTypeDef status;

  if (g_app_power_snapshot.probe_status != BQ25730_OK)
  {
    return BQ25730_ERROR;
  }

  status = BQ25730_SetChargeVoltageMv(&g_app_bq25730, APP_POWER_3S_CHARGE_VOLTAGE_MV);
  g_app_power_snapshot.last_i2c_status = (int32_t)status;
  if (status != BQ25730_OK)
  {
    AppPower_UpdateDebugGlobals();
    return (int32_t)status;
  }

  if (enabled == 0U)
  {
    current_ma = 0U;
  }

  status = BQ25730_SetChargeCurrentMa(&g_app_bq25730, current_ma);
  g_app_power_snapshot.last_i2c_status = (int32_t)status;
  AppPower_UpdateDebugGlobals();

  return (int32_t)status;
}

int32_t AppPower_RequestLowPowerMode(uint8_t enabled)
{
  g_app_power_snapshot.low_power_target = (enabled == 0U) ? 0U : 1U;
  g_app_power_snapshot.low_power_request_pending = 1U;
  AppPower_UpdateDebugGlobals();

  return (g_app_power_snapshot.probe_status == BQ25730_OK) ? BQ25730_OK : BQ25730_ERROR;
}
