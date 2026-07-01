#ifndef APP_POWER_H
#define APP_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APP_POWER_PACK_CAPACITY_MAH          2600U
#define APP_POWER_3S_CHARGE_VOLTAGE_MV       12600U
#define APP_POWER_3S_UNDERVOLTAGE_MV         9000U
#define APP_POWER_DEFAULT_CHARGE_CURRENT_MA  0U
#define APP_POWER_UNDERVOLTAGE_DEBOUNCE_MS   500U

typedef enum
{
  APP_POWER_STATE_UNKNOWN = 0,
  APP_POWER_STATE_IDLE,
  APP_POWER_STATE_CHARGING,
  APP_POWER_STATE_DISCHARGING,
  APP_POWER_STATE_OTG,
  APP_POWER_STATE_UNDERVOLTAGE,
  APP_POWER_STATE_FAULT
} AppPowerState_t;

#define APP_POWER_FLAG_BQ_PRESENT             0x00000001UL
#define APP_POWER_FLAG_ADC_VALID              0x00000002UL
#define APP_POWER_FLAG_UNDERVOLTAGE_RAW       0x00000004UL
#define APP_POWER_FLAG_UNDERVOLTAGE_CONFIRMED 0x00000008UL
#define APP_POWER_FLAG_CHARGER_FAULT          0x00000010UL
#define APP_POWER_FLAG_PROCHOT_HIGH           0x00000020UL
#define APP_POWER_FLAG_CHRG_OK_HIGH           0x00000040UL
#define APP_POWER_FLAG_OTG_VAP_HIGH           0x00000080UL
#define APP_POWER_FLAG_CMPOUT_HIGH            0x00000100UL
#define APP_POWER_FLAG_PG_HIGH                0x00000200UL

typedef struct
{
  uint32_t flags;
  uint32_t update_count;
  uint32_t battery_mv;
  uint32_t system_mv;
  uint32_t input_voltage_mv;
  uint32_t psys_mv;
  int32_t battery_current_ma;
  uint32_t charge_current_ma;
  uint32_t discharge_current_ma;
  uint32_t input_current_ma;
  uint32_t cmpin_mv;
  int32_t remaining_mah_x1000;
  uint32_t low_power_command_count;
  uint16_t charger_status;
  uint16_t prochot_status;
  uint16_t charge_option0;
  uint16_t charge_option1;
  uint16_t charge_option2;
  uint16_t charge_option3;
  uint16_t adc_option;
  uint16_t charge_voltage_reg;
  uint16_t charge_current_reg;
  uint16_t input_current_reg;
  uint16_t vsys_min_reg;
  uint16_t otg_voltage_reg;
  uint16_t otg_current_reg;
  uint32_t pin_state;
  int32_t init_status;
  int32_t probe_status;
  int32_t last_i2c_status;
  int32_t adc_status;
  int32_t pin_read_status;
  int32_t low_power_status;
  uint8_t manufacturer_id;
  uint8_t device_id;
  uint8_t battery_percent;
  uint8_t state;
  uint8_t low_power_enabled;
  uint8_t low_power_target;
  uint8_t low_power_request_pending;
  uint8_t adc_raw_vbat;
  uint8_t adc_raw_vsys;
  uint8_t adc_raw_psys;
  uint8_t adc_raw_vbus;
  uint8_t adc_raw_ichg;
  uint8_t adc_raw_idchg;
  uint8_t adc_raw_iin;
  uint8_t adc_raw_cmpin;
} AppPowerSnapshot_t;

extern volatile int32_t g_app_bq25730_init_status;
extern volatile int32_t g_app_bq25730_probe_status;
extern volatile int32_t g_app_bq25730_last_i2c_status;
extern volatile int32_t g_app_bq25730_adc_status;
extern volatile int32_t g_app_bq25730_pin_read_status;
extern volatile uint32_t g_app_bq25730_manufacturer_id;
extern volatile uint32_t g_app_bq25730_device_id;
extern volatile uint32_t g_app_bq25730_charger_status;
extern volatile uint32_t g_app_bq25730_prochot_status;
extern volatile uint32_t g_app_bq25730_pin_state;
extern volatile uint32_t g_app_bq25730_refresh_count;
extern volatile uint32_t g_app_bq25730_charge_option0;
extern volatile uint32_t g_app_bq25730_charge_option1;
extern volatile uint32_t g_app_bq25730_charge_option2;
extern volatile uint32_t g_app_bq25730_charge_option3;
extern volatile uint32_t g_app_bq25730_adc_option;
extern volatile uint32_t g_app_bq25730_charge_voltage_reg;
extern volatile uint32_t g_app_bq25730_charge_current_reg;
extern volatile uint32_t g_app_bq25730_input_current_reg;
extern volatile uint32_t g_app_bq25730_vsys_min_reg;
extern volatile uint32_t g_app_bq25730_otg_voltage_reg;
extern volatile uint32_t g_app_bq25730_otg_current_reg;
extern volatile uint32_t g_app_bq25730_adc_raw_vbat;
extern volatile uint32_t g_app_bq25730_adc_raw_vsys;
extern volatile uint32_t g_app_bq25730_adc_raw_psys;
extern volatile uint32_t g_app_bq25730_adc_raw_vbus;
extern volatile uint32_t g_app_bq25730_adc_raw_ichg;
extern volatile uint32_t g_app_bq25730_adc_raw_idchg;
extern volatile uint32_t g_app_bq25730_adc_raw_iin;
extern volatile uint32_t g_app_bq25730_adc_raw_cmpin;
extern volatile int32_t g_app_bq25730_low_power_status;
extern volatile uint32_t g_app_bq25730_low_power_command_count;
extern volatile uint32_t g_app_bq25730_low_power_enabled;
extern volatile uint32_t g_app_bq25730_low_power_target;
extern volatile uint32_t g_app_bq25730_low_power_request_pending;

extern volatile uint32_t g_app_power_flags;
extern volatile uint32_t g_app_power_state;
extern volatile uint32_t g_app_power_battery_mv;
extern volatile uint32_t g_app_power_system_mv;
extern volatile uint32_t g_app_power_input_voltage_mv;
extern volatile uint32_t g_app_power_psys_mv;
extern volatile uint32_t g_app_power_battery_percent;
extern volatile uint32_t g_app_power_cmpin_mv;
extern volatile int32_t g_app_power_battery_current_ma;
extern volatile int32_t g_app_power_remaining_mah_x1000;

void AppPower_Init(void);
void AppPower_Poll(uint32_t elapsed_ms);
void AppPower_GetSnapshot(AppPowerSnapshot_t *snapshot);
int32_t AppPower_SetCharging(uint8_t enabled, uint32_t current_ma);
int32_t AppPower_RequestLowPowerMode(uint8_t enabled);

#ifdef __cplusplus
}
#endif

#endif /* APP_POWER_H */
