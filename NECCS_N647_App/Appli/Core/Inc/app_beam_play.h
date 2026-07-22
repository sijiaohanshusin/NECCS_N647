/*
 * Beam audio playback: I2S2 master TX -> MAX98357A class-D amp (speaker).
 *
 * One 48 kHz mono int16 sink with two mutually exclusive producers:
 *  - live monitor: the beamformer tap (capture thread) mirrors each
 *    produced frame here, "hear where you point";
 *  - WAV playback: the media thread streams NECCS/AUDIO/AUDnnnnn.WAV.
 * WAV playback preempts the monitor; the monitor resumes when it ends.
 *
 * Wiring (expansion board P1 header, DuPont):
 *   BCLK = PF2 / I2S2_CK  (P1.17, AF5)
 *   LRC  = PC1 / I2S2_WS  (P1.19, AF5)
 *   DIN  = PG8 / I2S2_SDO (P1.21, AF5)
 *   VIN  = 5V (P6), GND = P6. GAIN/SD float (9 dB, (L+R)/2 output).
 * Pins are free in the .ioc (verified against the expansion netlist); like
 * the USB OTG bring-up, the peripheral is configured entirely here and
 * intentionally NOT added to CubeMX (avoids regeneration drift).
 */

#ifndef APP_BEAM_PLAY_H
#define APP_BEAM_PLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APP_BEAM_PLAY_SAMPLE_RATE_HZ 48000U

typedef struct
{
  uint8_t initialized;     /* I2S2 + DMA brought up                    */
  uint8_t monitor_on;      /* UI monitor switch state                  */
  uint8_t wav_active;      /* media thread owns the sink               */
  uint8_t streaming;       /* I2S DMA currently running                */
  uint32_t ring_level;     /* mono samples queued                      */
  uint32_t underruns;      /* half-buffers padded while a source active */
  uint32_t halves_out;     /* half-buffer services since stream start  */
  int32_t last_status;     /* last HAL/init status (0 = OK)            */
} AppBeamPlaySnapshot_t;

/* UI thread: live-monitor switch. Turning it on lazily brings up I2S2.
 * Returns 0 on success, negative HAL/init error otherwise. */
int32_t AppBeamPlay_SetMonitor(uint8_t on);
uint8_t AppBeamPlay_GetMonitor(void);

/* Capture thread: mirror one beamformed frame into the playback ring.
 * No-op unless the monitor is the active source. */
void AppBeamPlay_MonitorFeed(const int16_t *samples, uint32_t count);

/* Media thread: WAV playback session. Start takes the sink (monitor tap
 * pauses), Write feeds PCM, Stop releases it (monitor resumes if on). */
int32_t AppBeamPlay_WavStart(void);
void AppBeamPlay_WavStop(void);
uint32_t AppBeamPlay_WavWrite(const int16_t *samples, uint32_t count);
uint32_t AppBeamPlay_WavFreeSpace(void);
uint8_t AppBeamPlay_WavDrained(void);

void AppBeamPlay_GetSnapshot(AppBeamPlaySnapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* APP_BEAM_PLAY_H */
