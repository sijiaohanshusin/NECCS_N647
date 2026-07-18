/*
 * Pan/tilt gimbal + laser pointer driver (LD-1501MG / LD-3015MG PWM servos).
 *
 * Pin map audited against the REAL hardware netlists 2026-07-18
 * (Shared_References/Common_Modules: N6拓展板.net + Netlist_电源管理.net).
 * Signals travel over the 20-pin power FPC between the expansion board and
 * the power-management board, then out on its USB3-A / U8 GH-8 connectors:
 *
 *   FPC  power-board net -> harness   MCU pin  .ioc label / function
 *   ---  ----------------------------  ------  -----------------------------
 *   12   GPIO1 -> USB3-A pin5, U8-3    PE13    GIMBAL_PAN_PWM  (TIM1_CH3 AF1)
 *   13   GPIO2 -> USB3-A pin6, U8-4    PE14    GIMBAL_TILT_PWM (TIM1_CH4 AF1)
 *   11   ENB2  = U6 3.3 V laser LDO EN PC10    LASER_EN (rail gate, high=on)
 *   15   GPIO4 -> U8-6 ONLY            PD0     GIMBAL_RELAY_EN (active high)
 *   10   ENB1  = 7.4 V servo buck EN   PC7     GIMBAL_PWR_EN (rail off = low*)
 *        USB3-A pin1 / U8-1            -       7.4 V servo supply
 *        LASER/CN1/MIC/BQ XH + USB3-A pin9/U8-8  laser 3.3 V (U6 output)
 *
 *   Laser chain: 7.4 V rail (ENB1/PC7) -> U6 LDO in, ENB2/PC10 -> U6 EN.
 *   So laser-on implies rail-on; SetLaser handles that ordering. 2-wire
 *   laser modules plug into the U6-gated 3.3 V, no TTL line needed.
 *   PA12 (old TTL laser plan, FPC14/GPIO3) is released back to plain input.
 *   Hardware prerequisites for the U6 gating (doc §5): FPC wires 18/19 cut
 *   (split the two boards' 3.3 V), exp R14 pull-up removed, 100k pulldown
 *   on U6 EN, U4 not fitted, U6 ADJ divider set for 3.3 V out.
 *
 *   *EN polarity assumes the U5 buck disables when EN driven low; if the
 *    part has an inverted/auto-start EN, flip APP_GIMBAL_PWR_ON_LEVEL below.
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
  uint8_t power_on;      /* 7.4 V servo rail (power-board ENB1) */
  uint8_t laser_on;
  uint8_t relay_on;
  int16_t target_theta;  /* commanded acoustic angles, degrees */
  int16_t target_phi;
  uint16_t pan_pulse_us; /* live servo pulses (after slew limiting) */
  uint16_t tilt_pulse_us;
} AppGimbalSnapshot_t;

/* GPIO + TIM setup. Safe without hardware attached. Returns 0 on success. */
int32_t AppGimbal_Init(void);

/* Start/stop the gimbal: raises/drops the 7.4 V servo rail (power-board
 * ENB1 via GIMBAL_PWR_EN) and the servo PWM outputs together. Servos hold
 * position while enabled. */
void AppGimbal_SetEnabled(uint8_t enabled);

/* Point the gimbal at an acoustic direction (theta = azimuth, phi =
 * elevation, degrees, same convention as the localization pipeline).
 * Applies calibration mapping + slew-rate limiting internally. */
void AppGimbal_PointAt(int16_t theta_deg, int16_t phi_deg);

/* Laser rail gate (PC10/ENB2 -> power-board U6 LDO; laser-on also raises
 * the 7.4 V rail it feeds from) and auxiliary relay (PD0, U8 harness). */
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
