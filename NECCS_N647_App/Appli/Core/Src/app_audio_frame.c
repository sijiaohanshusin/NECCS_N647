#include "app_audio_frame.h"

#include <stddef.h>

static float App_AudioFrame_AbsF32(float value)
{
  return (value < 0.0f) ? -value : value;
}

static float App_AudioFrame_SqrtF32(float value)
{
  float x;

  if (value <= 0.0f)
  {
    return 0.0f;
  }

  x = (value > 1.0f) ? value : 1.0f;
  for (uint32_t i = 0U; i < 6U; i++)
  {
    x = 0.5f * (x + (value / x));
  }

  return x;
}

static AppAudioFrameStatus_t App_AudioFrame_PrepareCommon(AppAudioFrame_t *frame,
                                                          AppMicArrayMode_t mode,
                                                          AppAudioFrameFormat_t format,
                                                          uint32_t frame_len,
                                                          uint32_t samples_per_channel,
                                                          uint32_t seq,
                                                          uint64_t timestamp_us,
                                                          AppAudioFrameSource_t source)
{
  uint32_t channel_count;
  uint32_t sample_rate_hz;

  if ((frame == NULL) ||
      (frame_len == 0U) ||
      (samples_per_channel < frame_len) ||
      (App_MicArray_ValidateMode(mode) == 0U))
  {
    return APP_AUDIO_FRAME_INVALID_ARGUMENT;
  }

  channel_count = App_MicArray_GetModeMicCount(mode);
  sample_rate_hz = App_MicArray_GetModeSampleRateHz(mode);
  if ((channel_count == 0U) ||
      (channel_count > APP_AUDIO_FRAME_MAX_CHANNELS) ||
      (sample_rate_hz == 0U))
  {
    return APP_AUDIO_FRAME_UNSUPPORTED_MODE;
  }

  App_AudioFrame_Clear(frame);
  frame->mic_mode = mode;
  frame->source = source;
  frame->format = format;
  frame->sample_rate_hz = sample_rate_hz;
  frame->channel_count = channel_count;
  frame->frame_len = frame_len;
  frame->samples_per_channel = samples_per_channel;
  frame->seq = seq;
  frame->timestamp_us = timestamp_us;

  return APP_AUDIO_FRAME_OK;
}

static AppAudioFrameStatus_t App_AudioFrame_ComputeDiagnosticsI16(const AppAudioFrame_t *frame,
                                                                  AppAudioFrameChannelDiag_t *diag,
                                                                  uint32_t diag_capacity,
                                                                  uint32_t *diag_count)
{
  uint32_t channel_count = frame->channel_count;

  if (diag_capacity < channel_count)
  {
    return APP_AUDIO_FRAME_INVALID_ARGUMENT;
  }

  for (uint32_t channel = 0U; channel < channel_count; channel++)
  {
    const int16_t *samples = App_AudioFrame_GetChannelI16(frame, channel);
    const AppMicArrayMic_t *mic = App_MicArray_GetModeMic(frame->mic_mode, channel);
    float sum_sq = 0.0f;
    float sum_abs = 0.0f;
    float peak_abs = 0.0f;
    uint32_t clipped = 0U;

    if ((samples == NULL) || (mic == NULL))
    {
      return APP_AUDIO_FRAME_LAYOUT_MISMATCH;
    }

    for (uint32_t i = 0U; i < frame->frame_len; i++)
    {
      float sample = (float)samples[i] * APP_MIC_ARRAY_Q15_TO_FLOAT_SCALE;
      float abs_sample = App_AudioFrame_AbsF32(sample);

      sum_sq += sample * sample;
      sum_abs += abs_sample;
      if (abs_sample > peak_abs)
      {
        peak_abs = abs_sample;
      }
      if ((samples[i] >= APP_AUDIO_FRAME_CLIP_LIMIT_I16) ||
          (samples[i] <= -APP_AUDIO_FRAME_CLIP_LIMIT_I16))
      {
        clipped++;
      }
    }

    diag[channel].mic_id = mic->mic_id;
    diag[channel].channel_index = (uint8_t)channel;
    diag[channel].rms = App_AudioFrame_SqrtF32(sum_sq / (float)frame->frame_len);
    diag[channel].mean_abs = sum_abs / (float)frame->frame_len;
    diag[channel].peak_abs = peak_abs;
    diag[channel].clipped_samples = clipped;
    diag[channel].phase_suspect = 0U;
    diag[channel].bad_channel = 0U;
  }

  *diag_count = channel_count;
  return APP_AUDIO_FRAME_OK;
}

static AppAudioFrameStatus_t App_AudioFrame_ComputeDiagnosticsF32(const AppAudioFrame_t *frame,
                                                                  AppAudioFrameChannelDiag_t *diag,
                                                                  uint32_t diag_capacity,
                                                                  uint32_t *diag_count)
{
  uint32_t channel_count = frame->channel_count;

  if (diag_capacity < channel_count)
  {
    return APP_AUDIO_FRAME_INVALID_ARGUMENT;
  }

  for (uint32_t channel = 0U; channel < channel_count; channel++)
  {
    const float *samples = App_AudioFrame_GetChannelF32(frame, channel);
    const AppMicArrayMic_t *mic = App_MicArray_GetModeMic(frame->mic_mode, channel);
    float sum_sq = 0.0f;
    float sum_abs = 0.0f;
    float peak_abs = 0.0f;
    uint32_t clipped = 0U;

    if ((samples == NULL) || (mic == NULL))
    {
      return APP_AUDIO_FRAME_LAYOUT_MISMATCH;
    }

    for (uint32_t i = 0U; i < frame->frame_len; i++)
    {
      float sample = samples[i];
      float abs_sample = App_AudioFrame_AbsF32(sample);

      sum_sq += sample * sample;
      sum_abs += abs_sample;
      if (abs_sample > peak_abs)
      {
        peak_abs = abs_sample;
      }
      if (abs_sample >= APP_AUDIO_FRAME_CLIP_LIMIT_F32)
      {
        clipped++;
      }
    }

    diag[channel].mic_id = mic->mic_id;
    diag[channel].channel_index = (uint8_t)channel;
    diag[channel].rms = App_AudioFrame_SqrtF32(sum_sq / (float)frame->frame_len);
    diag[channel].mean_abs = sum_abs / (float)frame->frame_len;
    diag[channel].peak_abs = peak_abs;
    diag[channel].clipped_samples = clipped;
    diag[channel].phase_suspect = 0U;
    diag[channel].bad_channel = 0U;
  }

  *diag_count = channel_count;
  return APP_AUDIO_FRAME_OK;
}

void App_AudioFrame_Clear(AppAudioFrame_t *frame)
{
  if (frame == NULL)
  {
    return;
  }

  frame->mic_mode = APP_MIC_ARRAY_MODE_WIDE32_48K;
  frame->source = APP_AUDIO_FRAME_SOURCE_UNSPECIFIED;
  frame->format = APP_AUDIO_FRAME_FORMAT_NONE;
  frame->sample_rate_hz = 0U;
  frame->channel_count = 0U;
  frame->frame_len = 0U;
  frame->samples_per_channel = 0U;
  frame->seq = 0U;
  frame->timestamp_us = 0ULL;
  frame->drop_count = 0U;
  frame->error_count = 0U;
  frame->planar_i16 = NULL;
  frame->planar_f32 = NULL;
}

AppAudioFrameStatus_t App_AudioFrame_PreparePlanarI16(AppAudioFrame_t *frame,
                                                      AppMicArrayMode_t mode,
                                                      const int16_t *planar_i16,
                                                      uint32_t frame_len,
                                                      uint32_t samples_per_channel,
                                                      uint32_t seq,
                                                      uint64_t timestamp_us,
                                                      AppAudioFrameSource_t source)
{
  AppAudioFrameStatus_t status;

  if (planar_i16 == NULL)
  {
    return APP_AUDIO_FRAME_INVALID_ARGUMENT;
  }

  status = App_AudioFrame_PrepareCommon(frame,
                                        mode,
                                        APP_AUDIO_FRAME_FORMAT_PLANAR_I16,
                                        frame_len,
                                        samples_per_channel,
                                        seq,
                                        timestamp_us,
                                        source);
  if (status != APP_AUDIO_FRAME_OK)
  {
    return status;
  }

  frame->planar_i16 = planar_i16;
  return APP_AUDIO_FRAME_OK;
}

AppAudioFrameStatus_t App_AudioFrame_PreparePlanarF32(AppAudioFrame_t *frame,
                                                      AppMicArrayMode_t mode,
                                                      const float *planar_f32,
                                                      uint32_t frame_len,
                                                      uint32_t samples_per_channel,
                                                      uint32_t seq,
                                                      uint64_t timestamp_us,
                                                      AppAudioFrameSource_t source)
{
  AppAudioFrameStatus_t status;

  if (planar_f32 == NULL)
  {
    return APP_AUDIO_FRAME_INVALID_ARGUMENT;
  }

  status = App_AudioFrame_PrepareCommon(frame,
                                        mode,
                                        APP_AUDIO_FRAME_FORMAT_PLANAR_F32,
                                        frame_len,
                                        samples_per_channel,
                                        seq,
                                        timestamp_us,
                                        source);
  if (status != APP_AUDIO_FRAME_OK)
  {
    return status;
  }

  frame->planar_f32 = planar_f32;
  return APP_AUDIO_FRAME_OK;
}

AppAudioFrameStatus_t App_AudioFrame_FromTdmI16PlanarI16(AppAudioFrame_t *frame,
                                                         AppMicArrayMode_t mode,
                                                         const int16_t *bus_a_interleaved,
                                                         const int16_t *bus_b_interleaved,
                                                         uint32_t frame_len,
                                                         int16_t *dst_planar,
                                                         uint32_t dst_samples_per_channel,
                                                         uint32_t seq,
                                                         uint64_t timestamp_us)
{
  if (App_MicArray_DeinterleavePlanarI16(mode,
                                         bus_a_interleaved,
                                         bus_b_interleaved,
                                         frame_len,
                                         dst_planar,
                                         dst_samples_per_channel) != APP_MIC_ARRAY_OK)
  {
    return APP_AUDIO_FRAME_INVALID_ARGUMENT;
  }

  return App_AudioFrame_PreparePlanarI16(frame,
                                         mode,
                                         dst_planar,
                                         frame_len,
                                         dst_samples_per_channel,
                                         seq,
                                         timestamp_us,
                                         APP_AUDIO_FRAME_SOURCE_TDM_CAPTURE);
}

AppAudioFrameStatus_t App_AudioFrame_FromTdmI16PlanarF32(AppAudioFrame_t *frame,
                                                         AppMicArrayMode_t mode,
                                                         const int16_t *bus_a_interleaved,
                                                         const int16_t *bus_b_interleaved,
                                                         uint32_t frame_len,
                                                         float *dst_planar,
                                                         uint32_t dst_samples_per_channel,
                                                         uint32_t seq,
                                                         uint64_t timestamp_us,
                                                         float scale)
{
  if (App_MicArray_DeinterleavePlanarF32(mode,
                                         bus_a_interleaved,
                                         bus_b_interleaved,
                                         frame_len,
                                         dst_planar,
                                         dst_samples_per_channel,
                                         scale) != APP_MIC_ARRAY_OK)
  {
    return APP_AUDIO_FRAME_INVALID_ARGUMENT;
  }

  return App_AudioFrame_PreparePlanarF32(frame,
                                         mode,
                                         dst_planar,
                                         frame_len,
                                         dst_samples_per_channel,
                                         seq,
                                         timestamp_us,
                                         APP_AUDIO_FRAME_SOURCE_TDM_CAPTURE);
}

AppAudioFrameStatus_t App_AudioFrame_Validate(const AppAudioFrame_t *frame)
{
  uint32_t expected_channels;
  uint32_t expected_sample_rate;

  if ((frame == NULL) ||
      (frame->frame_len == 0U) ||
      (frame->samples_per_channel < frame->frame_len) ||
      (App_MicArray_ValidateMode(frame->mic_mode) == 0U))
  {
    return APP_AUDIO_FRAME_INVALID_ARGUMENT;
  }

  expected_channels = App_MicArray_GetModeMicCount(frame->mic_mode);
  expected_sample_rate = App_MicArray_GetModeSampleRateHz(frame->mic_mode);
  if ((frame->channel_count != expected_channels) ||
      (frame->sample_rate_hz != expected_sample_rate))
  {
    return APP_AUDIO_FRAME_LAYOUT_MISMATCH;
  }

  if ((frame->format == APP_AUDIO_FRAME_FORMAT_PLANAR_I16) &&
      (frame->planar_i16 != NULL) &&
      (frame->planar_f32 == NULL))
  {
    return APP_AUDIO_FRAME_OK;
  }

  if ((frame->format == APP_AUDIO_FRAME_FORMAT_PLANAR_F32) &&
      (frame->planar_f32 != NULL) &&
      (frame->planar_i16 == NULL))
  {
    return APP_AUDIO_FRAME_OK;
  }

  return APP_AUDIO_FRAME_LAYOUT_MISMATCH;
}

const int16_t *App_AudioFrame_GetChannelI16(const AppAudioFrame_t *frame,
                                            uint32_t channel_index)
{
  if ((frame == NULL) ||
      (frame->format != APP_AUDIO_FRAME_FORMAT_PLANAR_I16) ||
      (frame->planar_i16 == NULL) ||
      (channel_index >= frame->channel_count))
  {
    return NULL;
  }

  return &frame->planar_i16[channel_index * frame->samples_per_channel];
}

const float *App_AudioFrame_GetChannelF32(const AppAudioFrame_t *frame,
                                          uint32_t channel_index)
{
  if ((frame == NULL) ||
      (frame->format != APP_AUDIO_FRAME_FORMAT_PLANAR_F32) ||
      (frame->planar_f32 == NULL) ||
      (channel_index >= frame->channel_count))
  {
    return NULL;
  }

  return &frame->planar_f32[channel_index * frame->samples_per_channel];
}

AppAudioFrameStatus_t App_AudioFrame_ComputeDiagnostics(const AppAudioFrame_t *frame,
                                                        AppAudioFrameChannelDiag_t *diag,
                                                        uint32_t diag_capacity,
                                                        uint32_t *diag_count)
{
  AppAudioFrameStatus_t status;

  if ((diag == NULL) || (diag_count == NULL))
  {
    return APP_AUDIO_FRAME_INVALID_ARGUMENT;
  }

  *diag_count = 0U;
  status = App_AudioFrame_Validate(frame);
  if (status != APP_AUDIO_FRAME_OK)
  {
    return status;
  }

  if (frame->format == APP_AUDIO_FRAME_FORMAT_PLANAR_I16)
  {
    return App_AudioFrame_ComputeDiagnosticsI16(frame,
                                               diag,
                                               diag_capacity,
                                               diag_count);
  }

  if (frame->format == APP_AUDIO_FRAME_FORMAT_PLANAR_F32)
  {
    return App_AudioFrame_ComputeDiagnosticsF32(frame,
                                               diag,
                                               diag_capacity,
                                               diag_count);
  }

  return APP_AUDIO_FRAME_LAYOUT_MISMATCH;
}
