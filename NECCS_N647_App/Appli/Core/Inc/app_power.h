#ifndef APP_POWER_H
#define APP_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APP_POWER_PACK_CAPACITY_MAH          2600U
/* 4S1P Li-ion pack (integrated balance + protection board):
 * 16.8 V full, 12.0 V system cutoff (protection板 trips lower, ~10-11 V). */
#define APP_POWER_CHARGE_VOLTAGE_MV          16800U
#define APP_POWER_UNDERVOLTAGE_MV            12000U
#define APP_POWER_DEFAULT_CHARGE_CURRENT_MA  0U
/* 0.8C-ish ceiling for the 2600 mAh pack; BQ25730 LSB is 128 mA. */
#define APP_POWER_CHARGE_CURRENT_MAX_MA      2048U
/* BQ25730 charge watchdog is 175 s by default: rewrite ChargeCurrent well
 * inside that window while a nonzero charge request is active. */
#define APP_POWER_CHARGE_REFRESH_MS          60000U
#define APP_POWER_UNDERVOLTAGE_DEBOUNCE_MS   500U
/* BQ25730 loads VSYS_MIN from the CELL_BATPRESZ strap at adapter plug-in:
 * 12.3 V readback = strap decoded as 4S. Anything else means the R26/R27
 * divider is wrong (50% sits in the dead band between 2S and 3S) and
 * charging must stay blocked or SYSOVP will trip. */
#define APP_POWER_VSYS_MIN_4S_MV             12300U
/* CH224A PD sink shares I2C2 (7-bit 0x22/0x23), voltage register 0x0A:
 * 0=5V 1=9V 2=12V 3=15V 4=20V. Keep 12 V while VBUS TVS D2 is SMCJ15A
 * (15 V standoff) - only move to 4 (20 V) after D2 -> SMCJ22A. */
#define APP_POWER_PD_REQUEST_CODE            2U
/* Board sense resistors (power-board R2 = RAC, R5 = RSR). The corrected BOM
 * fits 10 mOhm shunts; set to 1 only if 5 mOhm parts are fitted instead. */
#define APP_POWER_SENSE_RES_5MOHM            0U

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
  int32_t battery_current_ma;
  uint32_t charge_current_ma;
  uint32_t discharge_current_ma;
  uint32_t input_current_ma;
  uint32_t cmpin_mv;
  int32_t remaining_mah_x1000;
  uint16_t charger_status;
  uint16_t prochot_status;
  uint32_t pin_state;
  int32_t init_status;
  int32_t probe_status;
  int32_t last_i2c_status;
  int32_t pin_read_status;
  uint8_t manufacturer_id;
  uint8_t device_id;
  uint8_t battery_percent;
  uint8_t state;
} AppPowerSnapshot_t;

extern volatile int32_t g_app_bq25730_init_status;
extern volatile int32_t g_app_bq25730_probe_status;
extern volatile int32_t g_app_bq25730_last_i2c_status;
extern volatile int32_t g_app_bq25730_pin_read_status;
extern volatile uint32_t g_app_bq25730_manufacturer_id;
extern volatile uint32_t g_app_bq25730_device_id;
extern volatile uint32_t g_app_bq25730_charger_status;
extern volatile uint32_t g_app_bq25730_prochot_status;
extern volatile uint32_t g_app_bq25730_pin_state;
extern volatile uint32_t g_app_bq25730_refresh_count;

extern volatile uint32_t g_app_power_flags;
extern volatile uint32_t g_app_power_state;
extern volatile uint32_t g_app_power_battery_mv;
extern volatile uint32_t g_app_power_system_mv;
extern volatile uint32_t g_app_power_battery_percent;
extern volatile uint32_t g_app_power_cmpin_mv;
extern volatile int32_t g_app_power_battery_current_ma;
extern volatile int32_t g_app_power_remaining_mah_x1000;

void AppPower_Init(void);
void AppPower_Poll(uint32_t elapsed_ms);
void AppPower_GetSnapshot(AppPowerSnapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* APP_POWER_H */
