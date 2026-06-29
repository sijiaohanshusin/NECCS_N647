#include "app_acoustic_synthetic.h"

#include "arm_math.h"

#include <stddef.h>

static float App_AcousticSynthetic_Clamp(float value, float min_value, float max_value)
{
  if (value < min_value)
  {
    return min_value;
  }

  if (value > max_value)
  {
    return max_value;
  }

  return value;
}

static uint32_t App_AcousticSynthetic_Hash(uint32_t value)
{
  value ^= value >> 16;
  value *= 0x7feb352dUL;
  value ^= value >> 15;
  value *= 0x846ca68bUL;
  value ^= value >> 16;

  return value;
}

static float App_AcousticSynthetic_Noise(uint32_t seq, uint32_t channel, uint32_t sample)
{
  uint32_t raw = App_AcousticSynthetic_Hash((seq * 73856093UL) ^
                                           (channel * 19349663UL) ^
                                           (sample * 83492791UL));
  uint32_t centered = raw & 0xFFFFU;

  return ((float)centered / 32767.5f) - 1.0f;
}

AppAcousticImagingStatus_t App_AcousticSynthetic_FillPlaneWave(const AppAcousticImagingConfig_t *config,
                                                               float theta_deg,
                                                               float phi_deg,
                                                               float frequency_hz,
                                                               float amplitude,
                                                               float noise_amplitude,
                                                               uint32_t seq,
                                                               float *dst_planar,
                                                               uint32_t samples_per_channel,
                                                               AppAudioFrame_t *frame)
{
  AppAcousticImagingStatus_t status;
  float sin_theta;
  float cos_theta_unused;
  float sin_phi;
  float cos_phi;
  float dir_x;
  float dir_y;
  uint64_t timestamp_us;

  if ((config == NULL) || (dst_planar == NULL) || (frequency_hz <= 0.0f))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  status = App_AcousticImaging_ValidateConfig(config);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  if (samples_per_channel < config->frame_len)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  arm_sin_cos_f32(theta_deg, &sin_theta, &cos_theta_unused);
  (void)cos_theta_unused;
  arm_sin_cos_f32(phi_deg, &sin_phi, &cos_phi);
  dir_x = sin_theta * cos_phi;
  dir_y = sin_phi;
  amplitude = App_AcousticSynthetic_Clamp(amplitude, 0.0f, 1.0f);
  noise_amplitude = App_AcousticSynthetic_Clamp(noise_amplitude, 0.0f, 1.0f);

  for (uint32_t channel = 0U; channel < config->channel_count; channel++)
  {
    const AppMicArrayMic_t *mic = App_MicArray_GetModeMic(config->mic_mode, channel);
    float x_m;
    float y_m;
    float delay_s;
    float *dst;

    if (mic == NULL)
    {
      return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
    }

    x_m = (float)mic->x_0p1mm * 0.0001f;
    y_m = (float)mic->y_0p1mm * 0.0001f;
    delay_s = -((x_m * dir_x) + (y_m * dir_y)) / APP_ACOUSTIC_IMAGING_SPEED_OF_SOUND_MPS;
    dst = &dst_planar[channel * samples_per_channel];

    for (uint32_t sample = 0U; sample < config->frame_len; sample++)
    {
      uint32_t absolute_sample = (seq * config->frame_len) + sample;
      float t_s = (float)absolute_sample / (float)config->sample_rate_hz;
      float phase_deg = 360.0f * frequency_hz * (t_s - delay_s);
      float sin_v;
      float cos_v;
      float value;

      arm_sin_cos_f32(phase_deg, &sin_v, &cos_v);
      (void)cos_v;

      value = amplitude * sin_v;
      if (noise_amplitude > 0.0f)
      {
        value += noise_amplitude * App_AcousticSynthetic_Noise(seq, channel, sample);
      }

      dst[sample] = App_AcousticSynthetic_Clamp(value, -1.0f, 1.0f);
    }

    for (uint32_t sample = config->frame_len; sample < samples_per_channel; sample++)
    {
      dst[sample] = 0.0f;
    }
  }

  if (frame == NULL)
  {
    return APP_ACOUSTIC_IMAGING_OK;
  }

  timestamp_us = ((uint64_t)seq * (uint64_t)config->frame_len * 1000000ULL) /
                 (uint64_t)config->sample_rate_hz;

  if (App_AudioFrame_PreparePlanarF32(frame,
                                      config->mic_mode,
                                      dst_planar,
                                      config->frame_len,
                                      samples_per_channel,
                                      seq,
                                      timestamp_us,
                                      APP_AUDIO_FRAME_SOURCE_SYNTHETIC) != APP_AUDIO_FRAME_OK)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  return APP_ACOUSTIC_IMAGING_OK;
}
