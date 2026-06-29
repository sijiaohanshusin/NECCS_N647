#include "app_acoustic_imaging.h"

#include <stddef.h>

static const int16_t s_coarse_angles_deg[APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE] =
{
  -60, -45, -30, -15, 0, 15, 30, 45, 60
};

static const float s_coarse_sin[APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE] =
{
  -0.8660254038f, -0.7071067812f, -0.5f, -0.2588190451f, 0.0f,
   0.2588190451f,  0.5f,  0.7071067812f,  0.8660254038f
};

static const float s_coarse_cos[APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE] =
{
  0.5f, 0.7071067812f, 0.8660254038f, 0.9659258263f, 1.0f,
  0.9659258263f, 0.8660254038f, 0.7071067812f, 0.5f
};

static uint8_t App_AcousticImaging_IsValidProfile(AppAcousticImagingProfile_t profile)
{
  return ((profile == APP_ACOUSTIC_IMAGING_PROFILE_FAST) ||
          (profile == APP_ACOUSTIC_IMAGING_PROFILE_BALANCED) ||
          (profile == APP_ACOUSTIC_IMAGING_PROFILE_QUALITY)) ? 1U : 0U;
}

static uint16_t App_AcousticImaging_GetDefaultPairCount(AppMicArrayMode_t mode,
                                                       AppAcousticImagingProfile_t profile)
{
  if (mode == APP_MIC_ARRAY_MODE_WIDE32_48K)
  {
    switch (profile)
    {
    case APP_ACOUSTIC_IMAGING_PROFILE_FAST:
      return APP_ACOUSTIC_IMAGING_WIDE32_FAST_PAIRS;
    case APP_ACOUSTIC_IMAGING_PROFILE_BALANCED:
      return APP_ACOUSTIC_IMAGING_WIDE32_BALANCED_PAIRS;
    case APP_ACOUSTIC_IMAGING_PROFILE_QUALITY:
      return APP_ACOUSTIC_IMAGING_WIDE32_QUALITY_PAIRS;
    default:
      return 0U;
    }
  }

  if (mode == APP_MIC_ARRAY_MODE_CORE16_192K)
  {
    switch (profile)
    {
    case APP_ACOUSTIC_IMAGING_PROFILE_FAST:
      return APP_ACOUSTIC_IMAGING_CORE16_FAST_PAIRS;
    case APP_ACOUSTIC_IMAGING_PROFILE_BALANCED:
      return APP_ACOUSTIC_IMAGING_CORE16_BALANCED_PAIRS;
    case APP_ACOUSTIC_IMAGING_PROFILE_QUALITY:
      return APP_ACOUSTIC_IMAGING_CORE16_QUALITY_PAIRS;
    default:
      return 0U;
    }
  }

  return 0U;
}

static uint8_t App_AcousticImaging_PairExists(const AppAcousticImagingPair_t *pairs,
                                              uint32_t pair_count,
                                              uint8_t mic_a,
                                              uint8_t mic_b)
{
  for (uint32_t i = 0U; i < pair_count; i++)
  {
    if ((pairs[i].mic_a == mic_a) && (pairs[i].mic_b == mic_b))
    {
      return 1U;
    }
  }

  return 0U;
}

static void App_AcousticImaging_InsertCandidate(AppAcousticImagingPair_t *pairs,
                                                uint32_t pair_limit,
                                                uint32_t *pair_count,
                                                const AppAcousticImagingPair_t *candidate)
{
  uint32_t insert_at = *pair_count;

  if (pair_limit == 0U)
  {
    return;
  }

  for (uint32_t i = 0U; i < *pair_count; i++)
  {
    if (candidate->baseline_sq_m2 > pairs[i].baseline_sq_m2)
    {
      insert_at = i;
      break;
    }
  }

  if ((*pair_count >= pair_limit) && (insert_at >= pair_limit))
  {
    return;
  }

  if (*pair_count < pair_limit)
  {
    (*pair_count)++;
  }

  for (uint32_t i = *pair_count - 1U; i > insert_at; i--)
  {
    pairs[i] = pairs[i - 1U];
  }

  pairs[insert_at] = *candidate;
}

static void App_AcousticImaging_CountPairCoverage(const AppAcousticImagingPair_t *pairs,
                                                  uint32_t pair_count,
                                                  uint32_t channel_count,
                                                  uint8_t *coverage)
{
  for (uint32_t channel = 0U; channel < channel_count; channel++)
  {
    coverage[channel] = 0U;
  }

  for (uint32_t i = 0U; i < pair_count; i++)
  {
    coverage[pairs[i].mic_a]++;
    coverage[pairs[i].mic_b]++;
  }
}

static uint8_t App_AcousticImaging_FindReplacementIndex(const AppAcousticImagingPair_t *pairs,
                                                        uint32_t pair_count,
                                                        const uint8_t *coverage,
                                                        uint32_t *replace_index)
{
  for (uint32_t i = pair_count; i > 0U; i--)
  {
    uint32_t index = i - 1U;
    if ((coverage[pairs[index].mic_a] > 1U) &&
        (coverage[pairs[index].mic_b] > 1U))
    {
      *replace_index = index;
      return 1U;
    }
  }

  return 0U;
}

static AppAcousticImagingPair_t App_AcousticImaging_MakePair(const AppMicArrayMic_t *mic_a,
                                                             const AppMicArrayMic_t *mic_b,
                                                             uint8_t index_a,
                                                             uint8_t index_b)
{
  AppAcousticImagingPair_t pair;

  pair.mic_a = index_a;
  pair.mic_b = index_b;
  pair.dx_m = ((float)mic_a->x_0p1mm - (float)mic_b->x_0p1mm) * 0.0001f;
  pair.dy_m = ((float)mic_a->y_0p1mm - (float)mic_b->y_0p1mm) * 0.0001f;
  pair.baseline_sq_m2 = (pair.dx_m * pair.dx_m) + (pair.dy_m * pair.dy_m);

  return pair;
}

static void App_AcousticImaging_RepairCoverage(const AppAcousticImagingConfig_t *config,
                                               AppAcousticImagingPair_t *pairs,
                                               uint32_t pair_count)
{
  uint8_t coverage[APP_MIC_ARRAY_PHYSICAL_MIC_COUNT];

  App_AcousticImaging_CountPairCoverage(pairs,
                                        pair_count,
                                        config->channel_count,
                                        coverage);

  for (uint32_t missing = 0U; missing < config->channel_count; missing++)
  {
    AppAcousticImagingPair_t best_pair;
    uint8_t found = 0U;
    uint32_t replace_index = 0U;

    if (coverage[missing] != 0U)
    {
      continue;
    }

    for (uint32_t other = 0U; other < config->channel_count; other++)
    {
      const AppMicArrayMic_t *mic_a;
      const AppMicArrayMic_t *mic_b;
      AppAcousticImagingPair_t candidate;
      uint8_t a;
      uint8_t b;

      if (other == missing)
      {
        continue;
      }

      a = (missing < other) ? (uint8_t)missing : (uint8_t)other;
      b = (missing < other) ? (uint8_t)other : (uint8_t)missing;
      if (App_AcousticImaging_PairExists(pairs, pair_count, a, b) != 0U)
      {
        continue;
      }

      mic_a = App_MicArray_GetModeMic(config->mic_mode, a);
      mic_b = App_MicArray_GetModeMic(config->mic_mode, b);
      if ((mic_a == NULL) || (mic_b == NULL))
      {
        continue;
      }

      candidate = App_AcousticImaging_MakePair(mic_a, mic_b, a, b);
      if ((found == 0U) || (candidate.baseline_sq_m2 > best_pair.baseline_sq_m2))
      {
        best_pair = candidate;
        found = 1U;
      }
    }

    if ((found != 0U) &&
        (App_AcousticImaging_FindReplacementIndex(pairs,
                                                  pair_count,
                                                  coverage,
                                                  &replace_index) != 0U))
    {
      coverage[pairs[replace_index].mic_a]--;
      coverage[pairs[replace_index].mic_b]--;
      pairs[replace_index] = best_pair;
      coverage[best_pair.mic_a]++;
      coverage[best_pair.mic_b]++;
    }
  }
}

static void App_AcousticImaging_SortPairs(AppAcousticImagingPair_t *pairs,
                                          uint32_t pair_count)
{
  for (uint32_t i = 1U; i < pair_count; i++)
  {
    AppAcousticImagingPair_t value = pairs[i];
    uint32_t j = i;

    while ((j > 0U) && (value.baseline_sq_m2 > pairs[j - 1U].baseline_sq_m2))
    {
      pairs[j] = pairs[j - 1U];
      j--;
    }

    pairs[j] = value;
  }
}

AppAcousticImagingStatus_t App_AcousticImaging_GetDefaultConfig(AppMicArrayMode_t mode,
                                                                AppAcousticImagingProfile_t profile,
                                                                AppAcousticImagingConfig_t *config)
{
  uint16_t pair_count;

  if ((config == NULL) ||
      (App_MicArray_ValidateMode(mode) == 0U) ||
      (App_AcousticImaging_IsValidProfile(profile) == 0U))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  pair_count = App_AcousticImaging_GetDefaultPairCount(mode, profile);
  if (pair_count == 0U)
  {
    return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
  }

  config->mic_mode = mode;
  config->profile = profile;
  config->sample_rate_hz = App_MicArray_GetModeSampleRateHz(mode);
  config->channel_count = App_MicArray_GetModeMicCount(mode);
  config->pair_count = pair_count;
  config->coarse_grid_size = APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE;
  config->coarse_angle_min_deg = -60;
  config->coarse_angle_max_deg = 60;
  config->fine_top_k = APP_ACOUSTIC_IMAGING_FINE_TOP_K;
  config->fine_grid_size = APP_ACOUSTIC_IMAGING_FINE_GRID_SIZE;
  config->fine_span_deg = 10.0f;
  config->ui_target_fps = 20U;
  config->ui_min_fps = 10U;

  if (mode == APP_MIC_ARRAY_MODE_WIDE32_48K)
  {
    config->frame_len = APP_AUDIO_FRAME_DEFAULT_WIDE32_FRAME_LEN;
    config->nfft = 256U;
    config->active_bin_start = 3U;
    config->active_bin_end = 42U;
  }
  else
  {
    config->frame_len = APP_AUDIO_FRAME_DEFAULT_CORE16_FRAME_LEN;
    config->nfft = 512U;
    config->active_bin_start = 11U;
    config->active_bin_end = 107U;
  }

  return App_AcousticImaging_ValidateConfig(config);
}

AppAcousticImagingStatus_t App_AcousticImaging_ValidateConfig(const AppAcousticImagingConfig_t *config)
{
  uint32_t max_pairs;

  if ((config == NULL) ||
      (App_MicArray_ValidateMode(config->mic_mode) == 0U) ||
      (App_AcousticImaging_IsValidProfile(config->profile) == 0U) ||
      (config->sample_rate_hz != App_MicArray_GetModeSampleRateHz(config->mic_mode)) ||
      (config->channel_count != App_MicArray_GetModeMicCount(config->mic_mode)) ||
      (config->channel_count == 0U) ||
      (config->frame_len == 0U) ||
      (config->nfft < config->frame_len) ||
      (config->active_bin_end < config->active_bin_start) ||
      (config->coarse_grid_size != APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE) ||
      (config->coarse_angle_min_deg != -60) ||
      (config->coarse_angle_max_deg != 60) ||
      (config->fine_top_k != APP_ACOUSTIC_IMAGING_FINE_TOP_K) ||
      (config->fine_grid_size != APP_ACOUSTIC_IMAGING_FINE_GRID_SIZE))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  max_pairs = (config->channel_count * (config->channel_count - 1U)) / 2U;
  if ((config->pair_count == 0U) ||
      (config->pair_count > max_pairs) ||
      (config->pair_count > APP_ACOUSTIC_IMAGING_PAIR_COUNT_MAX))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  return APP_ACOUSTIC_IMAGING_OK;
}

const char *App_AcousticImaging_ProfileName(AppAcousticImagingProfile_t profile)
{
  switch (profile)
  {
  case APP_ACOUSTIC_IMAGING_PROFILE_FAST:
    return "FAST";
  case APP_ACOUSTIC_IMAGING_PROFILE_BALANCED:
    return "BALANCED";
  case APP_ACOUSTIC_IMAGING_PROFILE_QUALITY:
    return "QUALITY";
  default:
    return "UNKNOWN";
  }
}

AppAcousticImagingStatus_t App_AcousticImaging_BuildPairSet(const AppAcousticImagingConfig_t *config,
                                                            AppAcousticImagingPair_t *pairs,
                                                            uint32_t pair_capacity,
                                                            uint32_t *pair_count)
{
  AppAcousticImagingStatus_t status;
  uint32_t selected_count = 0U;

  if ((pairs == NULL) || (pair_count == NULL))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  *pair_count = 0U;
  status = App_AcousticImaging_ValidateConfig(config);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  if (pair_capacity < config->pair_count)
  {
    return APP_ACOUSTIC_IMAGING_CAPACITY_TOO_SMALL;
  }

  for (uint32_t a = 0U; a < config->channel_count; a++)
  {
    for (uint32_t b = a + 1U; b < config->channel_count; b++)
    {
      const AppMicArrayMic_t *mic_a = App_MicArray_GetModeMic(config->mic_mode, a);
      const AppMicArrayMic_t *mic_b = App_MicArray_GetModeMic(config->mic_mode, b);
      AppAcousticImagingPair_t candidate;

      if ((mic_a == NULL) || (mic_b == NULL))
      {
        return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
      }

      candidate = App_AcousticImaging_MakePair(mic_a, mic_b, (uint8_t)a, (uint8_t)b);
      App_AcousticImaging_InsertCandidate(pairs,
                                          config->pair_count,
                                          &selected_count,
                                          &candidate);
    }
  }

  if (selected_count != config->pair_count)
  {
    return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
  }

  App_AcousticImaging_RepairCoverage(config, pairs, selected_count);
  App_AcousticImaging_SortPairs(pairs, selected_count);
  *pair_count = selected_count;

  return APP_ACOUSTIC_IMAGING_OK;
}

AppAcousticImagingStatus_t App_AcousticImaging_FillCoarseGrid(const AppAcousticImagingConfig_t *config,
                                                              float *theta_deg,
                                                              float *phi_deg,
                                                              uint32_t grid_capacity,
                                                              uint32_t *grid_count)
{
  AppAcousticImagingStatus_t status;
  uint32_t out_index = 0U;

  if ((theta_deg == NULL) || (phi_deg == NULL) || (grid_count == NULL))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  *grid_count = 0U;
  status = App_AcousticImaging_ValidateConfig(config);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  if (grid_capacity < APP_ACOUSTIC_IMAGING_COARSE_TOTAL)
  {
    return APP_ACOUSTIC_IMAGING_CAPACITY_TOO_SMALL;
  }

  for (uint32_t theta = 0U; theta < APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE; theta++)
  {
    for (uint32_t phi = 0U; phi < APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE; phi++)
    {
      theta_deg[out_index] = (float)s_coarse_angles_deg[theta];
      phi_deg[out_index] = (float)s_coarse_angles_deg[phi];
      out_index++;
    }
  }

  *grid_count = out_index;
  return APP_ACOUSTIC_IMAGING_OK;
}

AppAcousticImagingStatus_t App_AcousticImaging_FillCoarseTdoaLut(const AppAcousticImagingConfig_t *config,
                                                                 const AppAcousticImagingPair_t *pairs,
                                                                 uint32_t pair_count,
                                                                 float *tdoa_seconds,
                                                                 uint32_t tdoa_capacity,
                                                                 uint32_t *tdoa_count)
{
  AppAcousticImagingStatus_t status;
  uint32_t out_index = 0U;

  if ((pairs == NULL) || (tdoa_seconds == NULL) || (tdoa_count == NULL))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  *tdoa_count = 0U;
  status = App_AcousticImaging_ValidateConfig(config);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  if ((pair_count != config->pair_count) ||
      (tdoa_capacity < (APP_ACOUSTIC_IMAGING_COARSE_TOTAL * pair_count)))
  {
    return APP_ACOUSTIC_IMAGING_CAPACITY_TOO_SMALL;
  }

  for (uint32_t theta = 0U; theta < APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE; theta++)
  {
    float sin_theta = s_coarse_sin[theta];

    for (uint32_t phi = 0U; phi < APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE; phi++)
    {
      float cos_phi = s_coarse_cos[phi];
      float sin_phi = s_coarse_sin[phi];

      for (uint32_t pair = 0U; pair < pair_count; pair++)
      {
        tdoa_seconds[out_index] =
            ((pairs[pair].dx_m * sin_theta * cos_phi) +
             (pairs[pair].dy_m * sin_phi)) /
            APP_ACOUSTIC_IMAGING_SPEED_OF_SOUND_MPS;
        out_index++;
      }
    }
  }

  *tdoa_count = out_index;
  return APP_ACOUSTIC_IMAGING_OK;
}

AppAcousticImagingStatus_t App_AcousticImaging_ValidateFrame(const AppAcousticImagingConfig_t *config,
                                                             const AppAudioFrame_t *frame)
{
  AppAcousticImagingStatus_t config_status;
  AppAudioFrameStatus_t frame_status;

  config_status = App_AcousticImaging_ValidateConfig(config);
  if (config_status != APP_ACOUSTIC_IMAGING_OK)
  {
    return config_status;
  }

  frame_status = App_AudioFrame_Validate(frame);
  if (frame_status != APP_AUDIO_FRAME_OK)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  if ((frame->mic_mode != config->mic_mode) ||
      (frame->sample_rate_hz != config->sample_rate_hz) ||
      (frame->channel_count != config->channel_count) ||
      (frame->frame_len != config->frame_len))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  return APP_ACOUSTIC_IMAGING_OK;
}

void App_AcousticImaging_ClearVisFrame(AppAcousticImagingVisFrame_t *frame)
{
  if (frame == NULL)
  {
    return;
  }

  for (uint32_t i = 0U; i < APP_ACOUSTIC_IMAGING_GRID_TOTAL_MAX; i++)
  {
    frame->power[i] = 0.0f;
    frame->theta_deg[i] = 0.0f;
    frame->phi_deg[i] = 0.0f;
  }

  for (uint32_t i = 0U; i < APP_ACOUSTIC_IMAGING_VIS_CANDIDATE_MAX; i++)
  {
    frame->candidate[i].theta_deg = 0.0f;
    frame->candidate[i].phi_deg = 0.0f;
    frame->candidate[i].power = 0.0f;
    frame->candidate[i].quality = 0.0f;
    frame->candidate[i].contrast = 0.0f;
  }

  frame->grid_count = 0U;
  frame->peak_idx = 0U;
  frame->peak_value = 0.0f;
  frame->quality = 0.0f;
  frame->contrast = 0.0f;
  frame->active_profile = APP_ACOUSTIC_IMAGING_PROFILE_BALANCED;
  frame->mic_mode = APP_MIC_ARRAY_MODE_WIDE32_48K;
  frame->frame_seq = 0U;
  frame->candidate_count = 0U;
  frame->valid = 0U;
}
