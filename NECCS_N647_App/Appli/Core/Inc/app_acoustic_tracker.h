/*
 * Single-source direction tracker for the acoustic localization pipeline.
 *
 * Sits between the raw SRP estimates (one per processed frame, ~15-25 Hz)
 * and the UI. Replaces the old Model.cpp discrete gating (two-frame angle
 * agreement + 2 s freeze-hold), which at low estimate rates produced
 * 300-600 ms lock-in latency, missed one-frame transients entirely, and
 * either froze the crosshair for 2 s or teleported it (the reported
 * "crosshair lags the sound / points elsewhere").
 *
 * Architecture (ODAS-style estimate->track split):
 *  - per-axis constant-velocity Kalman filter on (theta, phi), measurement
 *    noise scaled by the SRP quality of each estimate
 *  - association gate with two-frame re-seed for genuine source jumps
 *  - confidence integrator (fast attack / slow decay) with display
 *    hysteresis instead of a hard freeze-hold
 *  - transient fast path: a single very strong estimate (clap) seeds the
 *    track and lights the display immediately
 *
 * Runs on the acoustic service thread; consumers read the tracked state
 * from the service snapshot.
 */

#ifndef APP_ACOUSTIC_TRACKER_H
#define APP_ACOUSTIC_TRACKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
  float theta_deg;      /* tracked azimuth   */
  float phi_deg;        /* tracked elevation */
  float vel_theta_dps;  /* tracked angular velocity (for render interp) */
  float vel_phi_dps;
  float confidence;     /* 0..1 integrated confidence */
  uint8_t display;      /* hysteresis-gated "show the source" bit */
  uint8_t seeded;       /* track holds a real position */
} AppAcousticTrackerState_t;

void AppAcousticTracker_Reset(void);

/* Feed one SRP output. valid=0 means "frame processed, nothing detected"
 * (drives coasting + confidence decay). quality_pct is the SRP prominence
 * in percent (snapshot units), dt_ms the time since the previous call. */
void AppAcousticTracker_Update(uint8_t valid,
                               float theta_deg,
                               float phi_deg,
                               uint8_t quality_pct,
                               uint32_t dt_ms);

void AppAcousticTracker_GetState(AppAcousticTrackerState_t *state);

#ifdef __cplusplus
}
#endif

#endif /* APP_ACOUSTIC_TRACKER_H */
