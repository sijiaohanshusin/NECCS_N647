/*
 * Realtime acoustic imaging service for the N647 Wide32 capture path.
 */

#ifndef APP_ACOUSTIC_SERVICE_H
#define APP_ACOUSTIC_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "app_acoustic_srp.h"
#include "tx_api.h"

#define APP_ACOUSTIC_SERVICE_CAMERA_HFOV_DEG  77.0f
#define APP_ACOUSTIC_SERVICE_CAMERA_VFOV_DEG  61.1f

/* Continuous heat field resampled from the SRP grid (camera aspect 4:3).
 * 96x72 keeps the bilinear upscale step at ~6.7 px per field cell, which
 * removes the visible blockiness the 48x36 field showed on the panel. */
#define APP_ACOUSTIC_SERVICE_FIELD_W          96U
#define APP_ACOUSTIC_SERVICE_FIELD_H          72U
#define APP_ACOUSTIC_SERVICE_FIELD_COUNT \
  (APP_ACOUSTIC_SERVICE_FIELD_W * APP_ACOUSTIC_SERVICE_FIELD_H)
#define APP_ACOUSTIC_SERVICE_CAND_MAX         3U

/* Display spectrum: log-scaled magnitudes of FFT bins 1..96 (Wide32:
 * 187.5 Hz per bin -> 18 kHz span). 64 -> 96 in 2026-07-20 band-mode
 * rework: leak-type sources peak ABOVE 12 kHz (demo hiss: 13.8 kHz), the
 * old panel could not even show them, let alone let the user drag the
 * band there. The NPU classifier keeps consuming the FIRST 64 slots
 * (AppNpu_FeedSpectrum reads exactly 64 - its training window). */
#define APP_ACOUSTIC_SERVICE_SPECTRUM_BINS    96U

/* Analysis band selection mode (Fluke ii900-style split):
 * AUTO   - the service watches the wideband spectrum and moves the SRP
 *          band onto persistent energy concentrations by itself.
 * MANUAL - the band is exactly where the user dragged it (spectrum-panel
 *          handles); no automatic repositioning. */
typedef enum
{
  APP_ACOUSTIC_BAND_MODE_AUTO = 0,
  APP_ACOUSTIC_BAND_MODE_MANUAL = 1
} AppAcousticBandMode_t;

typedef enum
{
  APP_ACOUSTIC_SERVICE_STATUS_OK = 0,
  APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME = -100,
  APP_ACOUSTIC_SERVICE_STATUS_COPY_FAILED = -101
} AppAcousticServiceStatus_t;

/* Application scenes: each packs an SRP frequency band plus a heat-field
 * rendering parameter bundle. Bands are clamped to what the Wide32/48k
 * front-end can observe (bins 3..42, 563..7875 Hz). */
typedef enum
{
  APP_ACOUSTIC_SCENE_GENERAL = 0,    /* full band, balanced rendering   */
  APP_ACOUSTIC_SCENE_LEAK = 1,       /* high band, crisp/fast           */
  APP_ACOUSTIC_SCENE_BEARING = 2,    /* mid band, smoothed              */
  APP_ACOUSTIC_SCENE_ELECTRICAL = 3, /* upper band, high contrast       */
  APP_ACOUSTIC_SCENE_COUNT = 4
} AppAcousticScene_t;

/* Runtime-adjustable heat-field rendering parameters. */
typedef struct
{
  float db_floor;        /* display dynamic window floor, [-30, -6] dB   */
  float gamma;           /* post-normalisation power curve, [0.5, 2.5]   */
  float noise_gate;      /* static floor as fraction of peak, [0, 0.6]   */
  float bg_gain;         /* adaptive background floor gain, [1.0, 3.0]   */
  uint8_t smooth_passes; /* separable 3x3 smoothing passes, 0..3         */
} AppAcousticFieldParams_t;

typedef struct
{
  uint8_t initialized;
  uint8_t running;
  uint8_t valid;
  uint8_t auto_degraded;
  AppAcousticImagingRunMode_t active_mode;
  AppAcousticImagingRunMode_t requested_mode;
  AppAcousticImagingProfile_t active_profile;
  AppAcousticImagingProfile_t requested_profile;
  AppAcousticImagingBinPolicy_t active_bin_policy;
  AppAcousticImagingBinPolicy_t requested_bin_policy;
  AppAcousticSrpBackend_t backend;
  int32_t last_status;
  int32_t service_status;
  uint32_t input_seq;
  uint32_t output_seq;
  uint32_t processed_frames;
  uint32_t skipped_frames;
  uint32_t failed_frames;
  uint32_t degraded_count;
  uint32_t active_channel_mask;
  uint32_t pair_count;
  uint32_t grid_count;
  uint16_t active_bin_count;
  uint16_t srp_ms_x100;
  uint16_t fps_x10;
  int16_t theta_deg;
  int16_t phi_deg;
  uint8_t quality_pct;
  uint8_t contrast_pct;
  uint8_t scene;
  /* AppMicArrayMode_t of the running pipeline + 1 while a Wide32<->Core16
   * switch is in flight (capture clock/SAI/PCMD restart). */
  uint8_t array_mode;
  uint8_t array_switching;
  int8_t temperature_c;
  uint16_t band_lo_hz;
  uint16_t band_hi_hz;
  /* Band selection mode (AppAcousticBandMode_t) + 1 while the auto tracker
   * has actually moved the band off the scene default (UI shows "自动·追踪"). */
  uint8_t band_mode;
  uint8_t band_auto_active;
  uint16_t speed_mps_x10;
  AppAcousticFieldParams_t field_params;
  uint8_t cand_count;
  int16_t cand_theta[APP_ACOUSTIC_SERVICE_CAND_MAX];
  int16_t cand_phi[APP_ACOUSTIC_SERVICE_CAND_MAX];
  uint8_t cand_strength[APP_ACOUSTIC_SERVICE_CAND_MAX];
  /* Tracked source (app_acoustic_tracker, updated once per SRP estimate):
   * the UI consumes THESE for the crosshair/card, not the raw per-frame
   * theta_deg/phi_deg above. */
  int16_t track_theta_deg;
  int16_t track_phi_deg;
  uint8_t track_conf_pct;
  uint8_t track_display;
  uint8_t spectrum[APP_ACOUSTIC_SERVICE_SPECTRUM_BINS];
  uint8_t spectrum_peak_bin;
  uint8_t field[APP_ACOUSTIC_SERVICE_FIELD_COUNT];
  uint8_t perf_load[5];
  AppAcousticSrpPerf_t perf;
} AppAcousticServiceSnapshot_t;

extern volatile uint32_t g_app_acoustic_service_disable_auto_degrade;

AppAcousticImagingStatus_t AppAcousticService_Init(void);

void AppAcousticService_ThreadEntry(ULONG thread_input);

void AppAcousticService_GetSnapshot(AppAcousticServiceSnapshot_t *snapshot);

AppAcousticImagingStatus_t AppAcousticService_SetProfile(AppAcousticImagingProfile_t profile);

AppAcousticImagingStatus_t AppAcousticService_SetMode(AppAcousticImagingRunMode_t mode);

AppAcousticImagingStatus_t AppAcousticService_SetBinPolicy(AppAcousticImagingBinPolicy_t policy);

/* Array mode (Wide32@48k / Core16@192k ultrasonic). Orchestrates the full
 * capture pipeline switch; progress is visible via snapshot.array_switching. */
AppAcousticImagingStatus_t AppAcousticService_SetArrayMode(AppMicArrayMode_t mode);
AppMicArrayMode_t AppAcousticService_GetArrayMode(void);

/* Scene preset: frequency band (SRP re-init) + field parameter bundle. */
AppAcousticImagingStatus_t AppAcousticService_SetScene(AppAcousticScene_t scene);

/* Custom analysis band in Hz (from the spectrum panel drag handles).
 * Wide32 accepts bins 3..96 (563 Hz..18 kHz), Core16 bins 11..107; the
 * width is additionally capped by the runtime pair*bin budget. Overrides
 * the scene band until the next scene change, and switches the band mode
 * to MANUAL. Triggers an SRP re-init. */
AppAcousticImagingStatus_t AppAcousticService_SetBandHz(uint16_t lo_hz, uint16_t hi_hz);

/* Band mode: AUTO re-enables the spectrum-driven band tracker and drops
 * any manual band; MANUAL freezes the band where it currently sits. */
AppAcousticImagingStatus_t AppAcousticService_SetBandMode(AppAcousticBandMode_t mode);
AppAcousticBandMode_t AppAcousticService_GetBandMode(void);

/* Ambient temperature -> speed of sound; triggers an SRP LUT re-init. */
AppAcousticImagingStatus_t AppAcousticService_SetTemperature(int8_t temperature_c);

/* Heat-field rendering parameters; applied from the next processed frame,
 * no re-init required. Values are clamped to their documented ranges. */
void AppAcousticService_SetFieldParams(const AppAcousticFieldParams_t *params);
void AppAcousticService_GetFieldParams(AppAcousticFieldParams_t *params);

#ifdef __cplusplus
}
#endif

#endif /* APP_ACOUSTIC_SERVICE_H */
