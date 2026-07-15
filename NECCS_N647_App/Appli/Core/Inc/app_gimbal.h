/*
 * Pan/tilt gimbal + laser pointer driver (LD-1501MG / LD-3015MG PWM servos).
 *
 * Final pin map, fixed 2026-07-15 (in the .ioc as labels, generated into
 * main.h; all four are EXP_GPIO harness pins so the current board can test
 * with flying leads before the new carrier PCB is made):
 *
 *   carrier USB3 connector          MCU pin   .ioc label / function
 *   ------------------------------  --------  -----------------------------
 *   GPIO1 (STDA_SSRX-)              PE13      GIMBAL_PAN_PWM  (TIM1_CH3 AF1)
 *   GPIO2 (STDA_SSRX+)              PE14      GIMBAL_TILT_PWM (TIM1_CH4 AF1)
 *   GPIO3 (STDA_SSTX-)              PA12      LASER_EN  (MOSFET, active high)
 *   GPIO4 (STDA_SSTX+)              PB1       GIMBAL_RELAY_EN (active high)
 *   VBUS                            -         7.4 V servo/laser supply
 *
 * Servos: 50 Hz frame, 500..2500 us pulse. Per-axis mapping (offset/scale/
 * invert/limits) lives in a calibration table to be tuned on hardware.
 * Until the gimbal arrives this module only has to compile and be safely
 * callable (no hardware attached = PWM into open pins).
 */

#ifndef APP_GIMBAL_H
#define APP_GIMBAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
  uint8_t initialized;
  uint8_t enabled;       /* PWM outputs running */
  uint8_t laser_on;
  uint8_t relay_on;
  int16_t target_theta;  /* commanded acoustic angles, degrees */
  int16_t target_phi;
  uint16_t pan_pulse_us; /* live servo pulses (after slew limiting) */
  uint16_t tilt_pulse_us;
} AppGimbalSnapshot_t;

/* GPIO + TIM setup. Safe without hardware attached. Returns 0 on success. */
int32_t AppGimbal_Init(void);

/* Start/stop the servo PWM outputs (servos hold position while running). */
void AppGimbal_SetEnabled(uint8_t enabled);

/* Point the gimbal at an acoustic direction (theta = azimuth, phi =
 * elevation, degrees, same convention as the localization pipeline).
 * Applies calibration mapping + slew-rate limiting internally. */
void AppGimbal_PointAt(int16_t theta_deg, int16_t phi_deg);

/* Laser / auxiliary relay control (GPIO3 / GPIO4 on the carrier). */
void AppGimbal_SetLaser(uint8_t on);
void AppGimbal_SetRelay(uint8_t on);

/* Slew-rate tick: call periodically (any thread, >= 20 Hz); advances the
 * live pulses toward the target at the configured max speed. */
void AppGimbal_Poll(void);

void AppGimbal_GetSnapshot(AppGimbalSnapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* APP_GIMBAL_H */
