/*
 * Directional recording: delay-and-sum beamformer over the 32-mic array.
 *
 * The capture thread feeds every published Wide32 frame; the module aligns
 * all channels onto the steered direction (integer-sample delays, the 94 mm
 * aperture needs at most +/-7 samples at 48 kHz), sums them into a mono
 * int16 stream and parks it in a lock-free ring. The media thread drains
 * the ring into a WAV file; the UI steers the beam and reads the level.
 */

#ifndef APP_BEAM_RECORD_H
#define APP_BEAM_RECORD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "app_audio_frame.h"

#define APP_BEAM_RECORD_SAMPLE_RATE_HZ 48000U

typedef struct
{
  uint8_t active;            /* beam compute engaged (aiming or recording) */
  uint8_t recording;         /* PCM flowing into the ring for the WAV writer */
  int16_t theta_deg;         /* applied steering */
  int16_t phi_deg;
  int8_t rms_dbfs;           /* beam output level, EMA, -90..0 */
  uint32_t frames_fed;
  uint32_t samples_produced;
  uint32_t ring_overruns;
} AppBeamRecordSnapshot_t;

/* UI: engage/disengage the beam compute (aiming mode). Cheap when idle. */
void AppBeamRecord_SetActive(uint8_t active);

/* UI: steer the beam (auto-track or manual drag both land here). */
void AppBeamRecord_SetSteering(int16_t theta_deg, int16_t phi_deg);

/* Media thread: gate PCM production. Enabling resets the ring. */
void AppBeamRecord_SetRecording(uint8_t recording);

/* Media thread: drain up to max_samples from the ring. Returns count. */
uint32_t AppBeamRecord_Read(int16_t *dst, uint32_t max_samples);

/* Capture thread: process one published audio frame (Wide32 only). */
void AppBeamRecord_FeedFrame(const AppAudioFrame_t *frame);

void AppBeamRecord_GetSnapshot(AppBeamRecordSnapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* APP_BEAM_RECORD_H */
