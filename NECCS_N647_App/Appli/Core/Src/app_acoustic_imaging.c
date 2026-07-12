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

static uint8_t App_AcousticImaging_IsValidRunMode(AppAcousticImagingRunMode_t mode)
{
  return ((mode == APP_ACOUSTIC_IMAGING_MODE_FAST) ||
          (mode == APP_ACOUSTIC_IMAGING_MODE_STANDARD) ||
          (mode == APP_ACOUSTIC_IMAGING_MODE_HIGH_QUALITY)) ? 1U : 0U;
}

static uint8_t App_AcousticImaging_IsValidAlgorithm(AppAcousticImagingAlgorithm_t algorithm)
{
  return ((algorithm == APP_ACOUSTIC_IMAGING_ALGO_WIDE32_FAST_SRP) ||
          (algorithm == APP_ACOUSTIC_IMAGING_ALGO_WIDE32_GENERAL_SRP) ||
          (algorithm == APP_ACOUSTIC_IMAGING_ALGO_WIDE32_QUALITY_SRP) ||
          (algorithm == APP_ACOUSTIC_IMAGING_ALGO_WIDE32_HF_HINT) ||
          (algorithm == APP_ACOUSTIC_IMAGING_ALGO_CORE16_HF_NEARFIELD)) ? 1U : 0U;
}

static uint8_t App_AcousticImaging_IsValidPairSelect(AppAcousticImagingPairSelect_t pair_select)
{
  return ((pair_select == APP_ACOUSTIC_IMAGING_PAIR_SELECT_LONG_BASELINE) ||
          (pair_select == APP_ACOUSTIC_IMAGING_PAIR_SELECT_SHORT_BASELINE)) ? 1U : 0U;
}

static uint8_t App_AcousticImaging_IsValidBinPolicy(AppAcousticImagingBinPolicy_t policy)
{
  return ((policy == APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT) ||
          (policy == APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B12) ||
          (policy == APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B16) ||
          (policy == APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B24) ||
          (policy == APP_ACOUSTIC_IMAGING_BIN_POLICY_QUALITY_B40)) ? 1U : 0U;
}

static void App_AcousticImaging_ClearBins(AppAcousticImagingConfig_t *config)
{
  if (config == NULL)
  {
    return;
  }

  config->active_bin_count = 0U;
  for (uint32_t i = 0U; i < APP_ACOUSTIC_IMAGING_BIN_COUNT_MAX; i++)
  {
    config->active_bins[i] = 0U;
  }
}

static AppAcousticImagingStatus_t App_AcousticImaging_SetContinuousBins(AppAcousticImagingConfig_t *config,
                                                                        uint16_t bin_start,
                                                                        uint16_t bin_end)
{
  uint32_t bin_count;

  if ((config == NULL) || (bin_end < bin_start))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  bin_count = ((uint32_t)bin_end - (uint32_t)bin_start) + 1U;
  if (bin_count > APP_ACOUSTIC_IMAGING_BIN_COUNT_MAX)
  {
    return APP_ACOUSTIC_IMAGING_CAPACITY_TOO_SMALL;
  }

  App_AcousticImaging_ClearBins(config);
  config->active_bin_start = bin_start;
  config->active_bin_end = bin_end;
  config->active_bin_count = (uint16_t)bin_count;
  for (uint32_t i = 0U; i < bin_count; i++)
  {
    config->active_bins[i] = (uint16_t)(bin_start + i);
  }

  return APP_ACOUSTIC_IMAGING_OK;
}

static AppAcousticImagingStatus_t App_AcousticImaging_SetSparseBins(AppAcousticImagingConfig_t *config,
                                                                    const uint16_t *bins,
                                                                    uint16_t bin_count)
{
  if ((config == NULL) || (bins == NULL) || (bin_count == 0U) ||
      (bin_count > APP_ACOUSTIC_IMAGING_BIN_COUNT_MAX))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  App_AcousticImaging_ClearBins(config);
  config->active_bin_count = bin_count;
  config->active_bin_start = bins[0];
  config->active_bin_end = bins[bin_count - 1U];
  for (uint32_t i = 0U; i < bin_count; i++)
  {
    config->active_bins[i] = bins[i];
  }

  return APP_ACOUSTIC_IMAGING_OK;
}

static uint32_t App_AcousticImaging_ChannelMaskForCount(uint32_t channel_count)
{
  if (channel_count >= 32U)
  {
    return APP_ACOUSTIC_IMAGING_ALL_CHANNELS_MASK;
  }

  if (channel_count == 0U)
  {
    return 0U;
  }

  return (1UL << channel_count) - 1UL;
}

static float App_AcousticImaging_SqrtF32(float value)
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
    if (candidate->selection_score > pairs[i].selection_score)
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
  uint32_t coverage_count = channel_count;

  if ((pairs == NULL) || (coverage == NULL))
  {
    return;
  }

  if (coverage_count > APP_MIC_ARRAY_PHYSICAL_MIC_COUNT)
  {
    coverage_count = APP_MIC_ARRAY_PHYSICAL_MIC_COUNT;
  }

  for (uint32_t channel = 0U; channel < coverage_count; channel++)
  {
    coverage[channel] = 0U;
  }

  for (uint32_t i = 0U; i < pair_count; i++)
  {
    if (pairs[i].mic_a < coverage_count)
    {
      coverage[pairs[i].mic_a]++;
    }
    if (pairs[i].mic_b < coverage_count)
    {
      coverage[pairs[i].mic_b]++;
    }
  }
}

static uint8_t App_AcousticImaging_FindReplacementIndex(const AppAcousticImagingPair_t *pairs,
                                                        uint32_t pair_count,
                                                        const uint8_t *coverage,
                                                        uint32_t channel_count,
                                                        uint32_t *replace_index)
{
  uint32_t coverage_count = channel_count;

  if ((pairs == NULL) || (coverage == NULL) || (replace_index == NULL))
  {
    return 0U;
  }

  if (coverage_count > APP_MIC_ARRAY_PHYSICAL_MIC_COUNT)
  {
    coverage_count = APP_MIC_ARRAY_PHYSICAL_MIC_COUNT;
  }

  for (uint32_t i = pair_count; i > 0U; i--)
  {
    uint32_t index = i - 1U;
    const uint8_t mic_a = pairs[index].mic_a;
    const uint8_t mic_b = pairs[index].mic_b;

    if ((mic_a >= coverage_count) || (mic_b >= coverage_count))
    {
      *replace_index = index;
      return 1U;
    }

    if ((coverage[mic_a] > 1U) && (coverage[mic_b] > 1U))
    {
      *replace_index = index;
      return 1U;
    }
  }

  return 0U;
}

static AppAcousticImagingPair_t App_AcousticImaging_MakePair(const AppMicArrayMic_t *mic_a,
                                                             const AppMicArrayMic_t *mic_b,
                                                             const AppAcousticImagingConfig_t *config,
                                                             uint8_t index_a,
                                                             uint8_t index_b)
{
  AppAcousticImagingPair_t pair;

  pair.mic_a = index_a;
  pair.mic_b = index_b;
  pair.dx_m = ((float)mic_a->x_0p1mm - (float)mic_b->x_0p1mm) * 0.0001f;
  pair.dy_m = ((float)mic_a->y_0p1mm - (float)mic_b->y_0p1mm) * 0.0001f;
  pair.baseline_sq_m2 = (pair.dx_m * pair.dx_m) + (pair.dy_m * pair.dy_m);
  pair.baseline_m = App_AcousticImaging_SqrtF32(pair.baseline_sq_m2);
  pair.weight = 1.0f;
  if ((config != NULL) &&
      (config->pair_select == APP_ACOUSTIC_IMAGING_PAIR_SELECT_SHORT_BASELINE))
  {
    pair.selection_score = -pair.baseline_sq_m2;
  }
  else
  {
    pair.selection_score = pair.baseline_sq_m2;
  }

  return pair;
}

static void App_AcousticImaging_RepairCoverage(const AppAcousticImagingConfig_t *config,
                                               AppAcousticImagingPair_t *pairs,
                                               uint32_t pair_count)
{
  uint8_t coverage[APP_MIC_ARRAY_PHYSICAL_MIC_COUNT];
  uint32_t channel_count;

  if ((config == NULL) ||
      (pairs == NULL) ||
      (pair_count == 0U) ||
      (config->channel_count == 0U) ||
      (config->channel_count > APP_MIC_ARRAY_PHYSICAL_MIC_COUNT))
  {
    return;
  }

  channel_count = config->channel_count;

  App_AcousticImaging_CountPairCoverage(pairs,
                                        pair_count,
                                        channel_count,
                                        coverage);

  for (uint32_t missing = 0U; missing < channel_count; missing++)
  {
    AppAcousticImagingPair_t best_pair;
    uint8_t found = 0U;
    uint32_t replace_index = 0U;

    if (coverage[missing] != 0U)
    {
      continue;
    }

    for (uint32_t other = 0U; other < channel_count; other++)
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

      candidate = App_AcousticImaging_MakePair(mic_a, mic_b, config, a, b);
      if ((found == 0U) || (candidate.selection_score > best_pair.selection_score))
      {
        best_pair = candidate;
        found = 1U;
      }
    }

    if ((found != 0U) &&
        (App_AcousticImaging_FindReplacementIndex(pairs,
                                                  pair_count,
                                                  coverage,
                                                  channel_count,
                                                  &replace_index) != 0U))
    {
      const uint8_t old_a = pairs[replace_index].mic_a;
      const uint8_t old_b = pairs[replace_index].mic_b;
      if ((old_a < channel_count) && (coverage[old_a] > 0U))
      {
        coverage[old_a]--;
      }
      if ((old_b < channel_count) && (coverage[old_b] > 0U))
      {
        coverage[old_b]--;
      }
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

    while ((j > 0U) && (value.selection_score > pairs[j - 1U].selection_score))
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
  AppAcousticImagingStatus_t status;

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
  config->run_mode = APP_ACOUSTIC_IMAGING_MODE_STANDARD;
  config->algorithm = APP_ACOUSTIC_IMAGING_ALGO_WIDE32_GENERAL_SRP;
  if (mode == APP_MIC_ARRAY_MODE_WIDE32_48K)
  {
    if (profile == APP_ACOUSTIC_IMAGING_PROFILE_FAST)
    {
      config->run_mode = APP_ACOUSTIC_IMAGING_MODE_FAST;
      config->algorithm = APP_ACOUSTIC_IMAGING_ALGO_WIDE32_FAST_SRP;
    }
    else if (profile == APP_ACOUSTIC_IMAGING_PROFILE_QUALITY)
    {
      config->run_mode = APP_ACOUSTIC_IMAGING_MODE_HIGH_QUALITY;
      config->algorithm = APP_ACOUSTIC_IMAGING_ALGO_WIDE32_QUALITY_SRP;
    }
  }
  else
  {
    config->algorithm = APP_ACOUSTIC_IMAGING_ALGO_CORE16_HF_NEARFIELD;
  }
  config->pair_select = APP_ACOUSTIC_IMAGING_PAIR_SELECT_LONG_BASELINE;
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
  config->channel_mask = App_AcousticImaging_ChannelMaskForCount(config->channel_count);
  config->bad_channel_mask = 0U;
  config->temperature_c = APP_ACOUSTIC_IMAGING_DEFAULT_TEMP_C;
  config->speed_of_sound_mps = APP_ACOUSTIC_IMAGING_SPEED_OF_SOUND_MPS;
  config->smoothing_alpha = APP_ACOUSTIC_IMAGING_DEFAULT_SMOOTHING_ALPHA;
  config->quality_min = 0.03f;
  config->energy_min = 0.02f;
  config->adaptive_profile_enable = 1U;

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

  status = App_AcousticImaging_SetBinPolicy(config,
                                            APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  return App_AcousticImaging_ValidateConfig(config);
}

AppAcousticImagingStatus_t App_AcousticImaging_GetDefaultAlgorithmConfig(AppAcousticImagingAlgorithm_t algorithm,
                                                                         AppAcousticImagingConfig_t *config)
{
  AppAcousticImagingStatus_t status;

  if ((config == NULL) ||
      (App_AcousticImaging_IsValidAlgorithm(algorithm) == 0U))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  switch (algorithm)
  {
  case APP_ACOUSTIC_IMAGING_ALGO_WIDE32_FAST_SRP:
    status = App_AcousticImaging_GetDefaultConfig(APP_MIC_ARRAY_MODE_WIDE32_48K,
                                                  APP_ACOUSTIC_IMAGING_PROFILE_FAST,
                                                  config);
    break;

  case APP_ACOUSTIC_IMAGING_ALGO_WIDE32_GENERAL_SRP:
    status = App_AcousticImaging_GetDefaultConfig(APP_MIC_ARRAY_MODE_WIDE32_48K,
                                                  APP_ACOUSTIC_IMAGING_PROFILE_BALANCED,
                                                  config);
    break;

  case APP_ACOUSTIC_IMAGING_ALGO_WIDE32_QUALITY_SRP:
    status = App_AcousticImaging_GetDefaultConfig(APP_MIC_ARRAY_MODE_WIDE32_48K,
                                                  APP_ACOUSTIC_IMAGING_PROFILE_QUALITY,
                                                  config);
    break;

  case APP_ACOUSTIC_IMAGING_ALGO_WIDE32_HF_HINT:
    status = App_AcousticImaging_GetDefaultConfig(APP_MIC_ARRAY_MODE_WIDE32_48K,
                                                  APP_ACOUSTIC_IMAGING_PROFILE_FAST,
                                                  config);
    if (status == APP_ACOUSTIC_IMAGING_OK)
    {
      config->algorithm = APP_ACOUSTIC_IMAGING_ALGO_WIDE32_HF_HINT;
      config->pair_select = APP_ACOUSTIC_IMAGING_PAIR_SELECT_SHORT_BASELINE;
      status = App_AcousticImaging_SetContinuousBins(config,
                                                     APP_ACOUSTIC_IMAGING_HF_HINT_BIN_START,
                                                     APP_ACOUSTIC_IMAGING_HF_HINT_BIN_END);
      if (status != APP_ACOUSTIC_IMAGING_OK)
      {
        break;
      }
      config->bin_policy = APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT;
      config->smoothing_alpha = 0.20f;
      config->quality_min = 0.05f;
      status = App_AcousticImaging_ValidateConfig(config);
    }
    break;

  case APP_ACOUSTIC_IMAGING_ALGO_CORE16_HF_NEARFIELD:
    status = App_AcousticImaging_GetDefaultConfig(APP_MIC_ARRAY_MODE_CORE16_192K,
                                                  APP_ACOUSTIC_IMAGING_PROFILE_BALANCED,
                                                  config);
    break;

  default:
    status = APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
    break;
  }

  if (status == APP_ACOUSTIC_IMAGING_OK)
  {
    config->algorithm = algorithm;
  }

  return status;
}

AppAcousticImagingStatus_t App_AcousticImaging_GetDefaultRunModeArrayConfig(AppAcousticImagingRunMode_t mode,
                                                                            AppMicArrayMode_t mic_mode,
                                                                            AppAcousticImagingConfig_t *config)
{
  AppAcousticImagingStatus_t status;
  AppAcousticImagingProfile_t profile;
  AppAcousticImagingBinPolicy_t bin_policy;

  if ((config == NULL) ||
      (App_AcousticImaging_IsValidRunMode(mode) == 0U) ||
      (App_MicArray_ValidateMode(mic_mode) == 0U))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  switch (mode)
  {
  case APP_ACOUSTIC_IMAGING_MODE_FAST:
    profile = APP_ACOUSTIC_IMAGING_PROFILE_FAST;
    bin_policy = APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B12;
    break;

  case APP_ACOUSTIC_IMAGING_MODE_HIGH_QUALITY:
    profile = APP_ACOUSTIC_IMAGING_PROFILE_QUALITY;
    bin_policy = APP_ACOUSTIC_IMAGING_BIN_POLICY_QUALITY_B40;
    break;

  case APP_ACOUSTIC_IMAGING_MODE_STANDARD:
  default:
    profile = APP_ACOUSTIC_IMAGING_PROFILE_BALANCED;
    bin_policy = APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B16;
    break;
  }

  status = App_AcousticImaging_GetDefaultConfig(mic_mode, profile, config);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  config->run_mode = mode;
  /* SetBinPolicy maps non-Wide32 array modes onto their continuous default
   * band internally; the Wide32 sparse tables stay as-is. */
  status = App_AcousticImaging_SetBinPolicy(config, bin_policy);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  config->run_mode = mode;
  return App_AcousticImaging_ValidateConfig(config);
}

AppAcousticImagingStatus_t App_AcousticImaging_GetDefaultRunModeConfig(AppAcousticImagingRunMode_t mode,
                                                                       AppAcousticImagingConfig_t *config)
{
  return App_AcousticImaging_GetDefaultRunModeArrayConfig(mode,
                                                          APP_MIC_ARRAY_MODE_WIDE32_48K,
                                                          config);
}

static uint8_t App_AcousticImaging_ConfigEnumsAndRangesOk(const AppAcousticImagingConfig_t *config)
{
  return ((App_MicArray_ValidateMode(config->mic_mode) != 0U) &&
          (App_AcousticImaging_IsValidAlgorithm(config->algorithm) != 0U) &&
          (App_AcousticImaging_IsValidProfile(config->profile) != 0U) &&
          (App_AcousticImaging_IsValidRunMode(config->run_mode) != 0U) &&
          (App_AcousticImaging_IsValidPairSelect(config->pair_select) != 0U) &&
          (App_AcousticImaging_IsValidBinPolicy(config->bin_policy) != 0U) &&
          (config->sample_rate_hz == App_MicArray_GetModeSampleRateHz(config->mic_mode)) &&
          (config->channel_count == App_MicArray_GetModeMicCount(config->mic_mode)) &&
          (config->channel_count != 0U) &&
          (config->frame_len != 0U) &&
          (config->nfft >= config->frame_len) &&
          (config->coarse_grid_size == APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE) &&
          (config->coarse_angle_min_deg == -60) &&
          (config->coarse_angle_max_deg == 60) &&
          (config->fine_top_k == APP_ACOUSTIC_IMAGING_FINE_TOP_K) &&
          (config->fine_grid_size == APP_ACOUSTIC_IMAGING_FINE_GRID_SIZE) &&
          (config->speed_of_sound_mps >= 300.0f) &&
          (config->speed_of_sound_mps <= 380.0f) &&
          (config->smoothing_alpha >= 0.0f) &&
          (config->smoothing_alpha <= 0.95f) &&
          (config->quality_min >= 0.0f) &&
          (config->quality_min <= 1.0f) &&
          (config->energy_min >= 0.0f) &&
          (config->energy_min <= 1.0f)) ? 1U : 0U;
}

static uint8_t App_AcousticImaging_ConfigAlgorithmModeOk(const AppAcousticImagingConfig_t *config)
{
  if (config->algorithm == APP_ACOUSTIC_IMAGING_ALGO_CORE16_HF_NEARFIELD)
  {
    return (config->mic_mode == APP_MIC_ARRAY_MODE_CORE16_192K) ? 1U : 0U;
  }

  return (config->mic_mode == APP_MIC_ARRAY_MODE_WIDE32_48K) ? 1U : 0U;
}

static uint8_t App_AcousticImaging_ConfigPairCountOk(const AppAcousticImagingConfig_t *config)
{
  uint32_t max_pairs = (config->channel_count * (config->channel_count - 1U)) / 2U;

  return ((config->pair_count != 0U) &&
          (config->pair_count <= max_pairs) &&
          (config->pair_count <= APP_ACOUSTIC_IMAGING_PAIR_COUNT_MAX)) ? 1U : 0U;
}

/* Active bins must stay inside [start, end], below nfft/2, be strictly
 * increasing, and fit the LUT capacity. */
static uint8_t App_AcousticImaging_ConfigActiveBinsOk(const AppAcousticImagingConfig_t *config)
{
  if ((config->active_bin_count == 0U) ||
      (config->active_bin_count > APP_ACOUSTIC_IMAGING_BIN_COUNT_MAX) ||
      (config->active_bin_end < config->active_bin_start) ||
      (config->active_bin_end >= (config->nfft / 2U)))
  {
    return 0U;
  }

  for (uint32_t i = 0U; i < config->active_bin_count; i++)
  {
    uint16_t bin = config->active_bins[i];

    if ((bin < config->active_bin_start) ||
        (bin > config->active_bin_end) ||
        (bin >= (config->nfft / 2U)))
    {
      return 0U;
    }

    if ((i > 0U) && (bin <= config->active_bins[i - 1U]))
    {
      return 0U;
    }
  }

  return 1U;
}

static uint8_t App_AcousticImaging_ConfigChannelMaskOk(const AppAcousticImagingConfig_t *config)
{
  uint32_t valid_channel_mask = App_AcousticImaging_ChannelMaskForCount(config->channel_count);

  return (((config->channel_mask & valid_channel_mask) != 0U) &&
          ((config->channel_mask & ~valid_channel_mask) == 0U) &&
          ((config->bad_channel_mask & ~valid_channel_mask) == 0U)) ? 1U : 0U;
}

AppAcousticImagingStatus_t App_AcousticImaging_ValidateConfig(const AppAcousticImagingConfig_t *config)
{
  if ((config == NULL) ||
      (App_AcousticImaging_ConfigEnumsAndRangesOk(config) == 0U) ||
      (App_AcousticImaging_ConfigAlgorithmModeOk(config) == 0U) ||
      (App_AcousticImaging_ConfigPairCountOk(config) == 0U) ||
      (App_AcousticImaging_ConfigActiveBinsOk(config) == 0U) ||
      (App_AcousticImaging_ConfigChannelMaskOk(config) == 0U))
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

const char *App_AcousticImaging_RunModeName(AppAcousticImagingRunMode_t mode)
{
  switch (mode)
  {
  case APP_ACOUSTIC_IMAGING_MODE_FAST:
    return "FAST";
  case APP_ACOUSTIC_IMAGING_MODE_STANDARD:
    return "STANDARD";
  case APP_ACOUSTIC_IMAGING_MODE_HIGH_QUALITY:
    return "HIGH_QUALITY";
  default:
    return "UNKNOWN";
  }
}

const char *App_AcousticImaging_AlgorithmName(AppAcousticImagingAlgorithm_t algorithm)
{
  switch (algorithm)
  {
  case APP_ACOUSTIC_IMAGING_ALGO_WIDE32_FAST_SRP:
    return "Wide32-Fast-SRP";
  case APP_ACOUSTIC_IMAGING_ALGO_WIDE32_GENERAL_SRP:
    return "Wide32-General-SRP";
  case APP_ACOUSTIC_IMAGING_ALGO_WIDE32_QUALITY_SRP:
    return "Wide32-Quality-SRP";
  case APP_ACOUSTIC_IMAGING_ALGO_WIDE32_HF_HINT:
    return "Wide32-HF-Hint";
  case APP_ACOUSTIC_IMAGING_ALGO_CORE16_HF_NEARFIELD:
    return "Core16-HF-Nearfield";
  default:
    return "UNKNOWN";
  }
}

const char *App_AcousticImaging_BinPolicyName(AppAcousticImagingBinPolicy_t policy)
{
  switch (policy)
  {
  case APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT:
    return "PROFILE_DEFAULT";
  case APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B12:
    return "STANDARD_B12";
  case APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B16:
    return "STANDARD_B16";
  case APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B24:
    return "STANDARD_B24";
  case APP_ACOUSTIC_IMAGING_BIN_POLICY_QUALITY_B40:
    return "QUALITY_B40";
  default:
    return "UNKNOWN";
  }
}

AppAcousticImagingBinPolicy_t App_AcousticImaging_ResolveBinPolicy(AppAcousticImagingProfile_t profile,
                                                                    AppAcousticImagingBinPolicy_t policy)
{
  if (policy != APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT)
  {
    return policy;
  }

  switch (profile)
  {
  case APP_ACOUSTIC_IMAGING_PROFILE_FAST:
    return APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B12;
  case APP_ACOUSTIC_IMAGING_PROFILE_QUALITY:
    return APP_ACOUSTIC_IMAGING_BIN_POLICY_QUALITY_B40;
  case APP_ACOUSTIC_IMAGING_PROFILE_BALANCED:
  default:
    return APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B16;
  }
}

AppAcousticImagingStatus_t App_AcousticImaging_SetBinPolicy(AppAcousticImagingConfig_t *config,
                                                            AppAcousticImagingBinPolicy_t policy)
{
  static const uint16_t standard_b12[] =
  {
    7U, 8U, 9U, 10U, 12U, 14U, 16U, 17U, 26U, 27U, 40U, 41U
  };
  static const uint16_t standard_b16[] =
  {
    6U, 7U, 8U, 9U, 10U, 12U, 14U, 16U,
    17U, 20U, 23U, 26U, 27U, 38U, 40U, 41U
  };
  static const uint16_t standard_b24[] =
  {
    3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U,
    12U, 14U, 16U, 17U, 19U, 20U, 23U, 24U,
    26U, 27U, 36U, 38U, 39U, 40U, 41U, 42U
  };
  AppAcousticImagingStatus_t status;

  if ((config == NULL) ||
      (App_AcousticImaging_IsValidProfile(config->profile) == 0U) ||
      (App_AcousticImaging_IsValidBinPolicy(policy) == 0U))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  if (config->mic_mode != APP_MIC_ARRAY_MODE_WIDE32_48K)
  {
    config->bin_policy = APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT;
    status = App_AcousticImaging_SetContinuousBins(config,
                                                  config->active_bin_start,
                                                  config->active_bin_end);
  }
  else
  {
    policy = App_AcousticImaging_ResolveBinPolicy(config->profile, policy);
    config->bin_policy = policy;

    switch (policy)
    {
    case APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B12:
      status = App_AcousticImaging_SetSparseBins(config,
                                                 standard_b12,
                                                 (uint16_t)(sizeof(standard_b12) / sizeof(standard_b12[0])));
      break;

    case APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B16:
      status = App_AcousticImaging_SetSparseBins(config,
                                                 standard_b16,
                                                 (uint16_t)(sizeof(standard_b16) / sizeof(standard_b16[0])));
      break;

    case APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B24:
      status = App_AcousticImaging_SetSparseBins(config,
                                                 standard_b24,
                                                 (uint16_t)(sizeof(standard_b24) / sizeof(standard_b24[0])));
      break;

    case APP_ACOUSTIC_IMAGING_BIN_POLICY_QUALITY_B40:
      status = App_AcousticImaging_SetContinuousBins(config, 3U, 42U);
      break;

    default:
      status = APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
      break;
    }
  }

  /* Bin tables were just written by SetContinuousBins/SetSparseBins; the
   * config as a whole is validated once by the entry points
   * (GetDefault*Config / App_AcousticSrp_Init) instead of after every
   * setter. */
  return status;
}

AppAcousticImagingStatus_t App_AcousticImaging_SetTemperature(AppAcousticImagingConfig_t *config,
                                                              float temperature_c)
{
  float speed_of_sound_mps;

  if (config == NULL)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  /* Only the derived speed of sound needs checking here; the surrounding
   * config is validated at the entry points. */
  speed_of_sound_mps = 331.3f + (0.606f * temperature_c);
  if ((speed_of_sound_mps < 300.0f) || (speed_of_sound_mps > 380.0f))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  config->temperature_c = temperature_c;
  config->speed_of_sound_mps = speed_of_sound_mps;
  return APP_ACOUSTIC_IMAGING_OK;
}

AppAcousticImagingStatus_t App_AcousticImaging_SetBand(AppAcousticImagingConfig_t *config,
                                                       uint16_t bin_lo,
                                                       uint16_t bin_hi)
{
  if ((config == NULL) ||
      (bin_lo == 0U) ||
      (bin_hi < bin_lo) ||
      (bin_hi >= (config->nfft / 2U)))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  config->bin_policy = APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT;
  return App_AcousticImaging_SetContinuousBins(config, bin_lo, bin_hi);
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

      candidate = App_AcousticImaging_MakePair(mic_a, mic_b, config, (uint8_t)a, (uint8_t)b);
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
            config->speed_of_sound_mps;
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
  AppAudioFrameStatus_t frame_status;

  /* Called per frame from the SRP hot path: the config belongs to an
   * initialized context and was fully validated at init, so only the frame
   * and its match against the config are checked here. */
  if (config == NULL)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
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
  frame->algorithm = APP_ACOUSTIC_IMAGING_ALGO_WIDE32_GENERAL_SRP;
  frame->active_profile = APP_ACOUSTIC_IMAGING_PROFILE_BALANCED;
  frame->mic_mode = APP_MIC_ARRAY_MODE_WIDE32_48K;
  frame->frame_seq = 0U;
  frame->candidate_count = 0U;
  frame->valid = 0U;
}
