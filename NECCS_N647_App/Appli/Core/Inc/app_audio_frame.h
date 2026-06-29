/*
 * Hardware-neutral audio frame contract for acoustic imaging.
 *
 * Producers may be DMA callbacks, a synthetic source, or an offline replay.
 * Consumers see only planar microphone samples and frame metadata.
 */

#ifndef APP_AUDIO_FRAME_H
#define APP_AUDIO_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "app_mic_array.h"

#define APP_AUDIO_FRAME_MAX_CHANNELS              APP_MIC_ARRAY_PHYSICAL_MIC_COUNT
#define APP_AUDIO_FRAME_DEFAULT_WIDE32_FRAME_LEN  256U
#define APP_AUDIO_FRAME_DEFAULT_CORE16_FRAME_LEN  512U
#define APP_AUDIO_FRAME_CLIP_LIMIT_I16            32760
#define APP_AUDIO_FRAME_CLIP_LIMIT_F32            0.999f

typedef enum
{
  APP_AUDIO_FRAME_OK = 0,
  APP_AUDIO_FRAME_INVALID_ARGUMENT = -1,
  APP_AUDIO_FRAME_UNSUPPORTED_MODE = -2,
  APP_AUDIO_FRAME_LAYOUT_MISMATCH = -3
} AppAudioFrameStatus_t;

typedef enum
{
  APP_AUDIO_FRAME_FORMAT_NONE = 0,
  APP_AUDIO_FRAME_FORMAT_PLANAR_I16 = 1,
  APP_AUDIO_FRAME_FORMAT_PLANAR_F32 = 2
} AppAudioFrameFormat_t;

typedef enum
{
  APP_AUDIO_FRAME_SOURCE_UNSPECIFIED = 0,
  APP_AUDIO_FRAME_SOURCE_SYNTHETIC = 1,
  APP_AUDIO_FRAME_SOURCE_TDM_CAPTURE = 2,
  APP_AUDIO_FRAME_SOURCE_REPLAY = 3
} AppAudioFrameSource_t;

typedef struct
{
  AppMicArrayMode_t mic_mode;
  AppAudioFrameSource_t source;
  AppAudioFrameFormat_t format;
  uint32_t sample_rate_hz;
  uint32_t channel_count;
  uint32_t frame_len;
  uint32_t samples_per_channel;
  uint32_t seq;
  uint64_t timestamp_us;
  uint32_t drop_count;
  uint32_t error_count;
  const int16_t *planar_i16;
  const float *planar_f32;
} AppAudioFrame_t;

typedef struct
{
  uint8_t mic_id;
  uint8_t channel_index;
  float rms;
  float mean_abs;
  float peak_abs;
  uint32_t clipped_samples;
  uint8_t phase_suspect;
  uint8_t bad_channel;
} AppAudioFrameChannelDiag_t;

void App_AudioFrame_Clear(AppAudioFrame_t *frame);

AppAudioFrameStatus_t App_AudioFrame_PreparePlanarI16(AppAudioFrame_t *frame,
                                                      AppMicArrayMode_t mode,
                                                      const int16_t *planar_i16,
                                                      uint32_t frame_len,
                                                      uint32_t samples_per_channel,
                                                      uint32_t seq,
                                                      uint64_t timestamp_us,
                                                      AppAudioFrameSource_t source);

AppAudioFrameStatus_t App_AudioFrame_PreparePlanarF32(AppAudioFrame_t *frame,
                                                      AppMicArrayMode_t mode,
                                                      const float *planar_f32,
                                                      uint32_t frame_len,
                                                      uint32_t samples_per_channel,
                                                      uint32_t seq,
                                                      uint64_t timestamp_us,
                                                      AppAudioFrameSource_t source);

AppAudioFrameStatus_t App_AudioFrame_FromTdmI16PlanarI16(AppAudioFrame_t *frame,
                                                         AppMicArrayMode_t mode,
                                                         const int16_t *bus_a_interleaved,
                                                         const int16_t *bus_b_interleaved,
                                                         uint32_t frame_len,
                                                         int16_t *dst_planar,
                                                         uint32_t dst_samples_per_channel,
                                                         uint32_t seq,
                                                         uint64_t timestamp_us);

AppAudioFrameStatus_t App_AudioFrame_FromTdmI16PlanarF32(AppAudioFrame_t *frame,
                                                         AppMicArrayMode_t mode,
                                                         const int16_t *bus_a_interleaved,
                                                         const int16_t *bus_b_interleaved,
                                                         uint32_t frame_len,
                                                         float *dst_planar,
                                                         uint32_t dst_samples_per_channel,
                                                         uint32_t seq,
                                                         uint64_t timestamp_us,
                                                         float scale);

AppAudioFrameStatus_t App_AudioFrame_Validate(const AppAudioFrame_t *frame);

const int16_t *App_AudioFrame_GetChannelI16(const AppAudioFrame_t *frame,
                                            uint32_t channel_index);

const float *App_AudioFrame_GetChannelF32(const AppAudioFrame_t *frame,
                                          uint32_t channel_index);

AppAudioFrameStatus_t App_AudioFrame_ComputeDiagnostics(const AppAudioFrame_t *frame,
                                                        AppAudioFrameChannelDiag_t *diag,
                                                        uint32_t diag_capacity,
                                                        uint32_t *diag_count);

#ifdef __cplusplus
}
#endif

#endif /* APP_AUDIO_FRAME_H */
