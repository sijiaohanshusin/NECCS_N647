#include "app_acoustic_service.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "app_pcmd_capture.h"
#include "app_bringup_thread.h"
#include "app_npu.h"
#include "main.h"

#define APP_ACOUSTIC_SERVICE_TARGET_FPS          20U
#define APP_ACOUSTIC_SERVICE_MIN_FPS             10U
#define APP_ACOUSTIC_SERVICE_DEGRADE_LIMIT       3U
#define APP_ACOUSTIC_SERVICE_PROCESS_PERIOD_TICKS \
  (TX_TIMER_TICKS_PER_SECOND / APP_ACOUSTIC_SERVICE_TARGET_FPS)
/* The field is normalised honestly from real SRP data; whether an overlay is
 * shown for not-yet-valid frames is decided by the UI Model (DEBUG preview)
 * and the display alpha policy, never by faking confidence here. */
#define APP_ACOUSTIC_SERVICE_SAMPLE_COUNT \
  (APP_MIC_ARRAY_PHYSICAL_MIC_COUNT * APP_AUDIO_FRAME_DEFAULT_WIDE32_FRAME_LEN)
#define APP_ACOUSTIC_SERVICE_CAMERA_HFOV_HALF_DEG \
  (APP_ACOUSTIC_SERVICE_CAMERA_HFOV_DEG * 0.5f)
#define APP_ACOUSTIC_SERVICE_CAMERA_VFOV_HALF_DEG \
  (APP_ACOUSTIC_SERVICE_CAMERA_VFOV_DEG * 0.5f)

/* Heat-field rendering defaults; the live values sit in s_field_params and
 * are runtime-adjustable (scene presets / settings page). Adaptive
 * background subtraction (bg_gain, H7-style noise_adapt_gain): the display
 * floor is the field mean scaled by that gain, so a diffuse ambient field
 * stays dark and only sources standing out of the background light up. */
#define APP_ACOUSTIC_SERVICE_FIELD_EMA_ATTACK   0.65f
#define APP_ACOUSTIC_SERVICE_FIELD_EMA_DECAY    0.12f
#define APP_ACOUSTIC_SERVICE_FIELD_FINE_GAIN    0.65f
/* Sigma/radius are in field cells; scaled with the 96x72 field density so
 * the angular footprint of a fine bump stays the same as before. */
#define APP_ACOUSTIC_SERVICE_FIELD_FINE_SIGMA   2.7f
/* Frequency resolution of the Wide32/48k pipeline (48000 / 256). */
#define APP_ACOUSTIC_SERVICE_BIN_HZ             187.5f
#define APP_ACOUSTIC_SERVICE_TEMP_MIN_C         (-20)
#define APP_ACOUSTIC_SERVICE_TEMP_MAX_C         60

static AppAcousticSrpContext_t s_srp_ctx __attribute__((aligned(32)));
/* CPU-only structures: cached external RAM keeps internal SRAM free for the
 * hot paths (the 96x72 field grew these by ~10 KB). GDB symbol names are
 * unchanged (debug scripts reference them by name, not by address). */
static AppAcousticImagingVisFrame_t s_vis_frame
    __attribute__((section(".EXTRAM"), aligned(32)));
static AppAcousticServiceSnapshot_t s_snapshot
    __attribute__((section(".EXTRAM"), aligned(32)));
static float s_service_samples[APP_ACOUSTIC_SERVICE_SAMPLE_COUNT]
    __attribute__((section(".EXTRAM"), aligned(32)));

/* Heat-field working state (service thread only; CPU-only access, so the
 * cached external RAM is fine and keeps internal SRAM for code). */
static float s_field_work[APP_ACOUSTIC_SERVICE_FIELD_COUNT]
    __attribute__((section(".EXTRAM"), aligned(32)));
static float s_field_smooth[APP_ACOUSTIC_SERVICE_FIELD_COUNT]
    __attribute__((section(".EXTRAM"), aligned(32)));
static float s_field_accum[APP_ACOUSTIC_SERVICE_FIELD_COUNT]
    __attribute__((section(".EXTRAM"), aligned(32)));
static float s_field_peak_ema;

volatile uint32_t g_app_acoustic_service_disable_auto_degrade;

static volatile AppAcousticImagingRunMode_t s_requested_mode = APP_ACOUSTIC_IMAGING_MODE_STANDARD;
static volatile AppAcousticImagingProfile_t s_requested_profile = APP_ACOUSTIC_IMAGING_PROFILE_BALANCED;
static volatile AppAcousticImagingBinPolicy_t s_requested_bin_policy =
    APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT;
static volatile uint8_t s_profile_change_pending;
static AppAcousticImagingRunMode_t s_active_mode = APP_ACOUSTIC_IMAGING_MODE_STANDARD;
static AppAcousticImagingProfile_t s_active_profile = APP_ACOUSTIC_IMAGING_PROFILE_BALANCED;

/* Scene / temperature / field-parameter runtime state. Scene band presets
 * are clamped to the Wide32/48k observable range (bins 3..42). bin_lo==0
 * means "keep the profile's default bin policy" (the verified baseline). */
typedef struct
{
  uint16_t bin_lo;
  uint16_t bin_hi;
  AppAcousticFieldParams_t params;
} AppAcousticScenePreset_t;

static const AppAcousticScenePreset_t s_scene_presets[APP_ACOUSTIC_SCENE_COUNT] =
{
  /* GENERAL: policy-default bins, balanced rendering (verified baseline). */
  { 0U, 0U,   { -15.0f, 1.10f, 0.10f, 1.45f, 2U } },
  /* LEAK: high band, crisp and responsive. */
  { 27U, 42U, { -12.0f, 1.25f, 0.06f, 1.30f, 1U } },
  /* BEARING: mid band, extra smoothing for steady hot-spots. */
  { 11U, 32U, { -18.0f, 1.15f, 0.10f, 1.50f, 3U } },
  /* ELECTRICAL: upper band, high contrast. */
  { 16U, 42U, { -14.0f, 1.35f, 0.08f, 1.40f, 2U } },
};

static volatile AppAcousticScene_t s_requested_scene = APP_ACOUSTIC_SCENE_GENERAL;
static AppAcousticScene_t s_active_scene = APP_ACOUSTIC_SCENE_GENERAL;
static volatile int8_t s_requested_temperature_c = 25;
static int8_t s_active_temperature_c = 25;
/* Custom analysis band (bins) from the spectrum panel; 0 = use scene band.
 * Cleared on scene change. */
static volatile uint16_t s_requested_band_lo_bin;
static volatile uint16_t s_requested_band_hi_bin;
static uint16_t s_active_band_lo_bin;
static uint16_t s_active_band_hi_bin;
static AppAcousticFieldParams_t s_field_params = { -15.0f, 1.10f, 0.10f, 1.45f, 2U };
/* Copy used by the processing path for the current frame. */
static AppAcousticFieldParams_t s_field_active;
static uint8_t s_have_input_seq;
static uint32_t s_last_input_seq;
static uint32_t s_over_budget_count;
static uint32_t s_error_count;
static uint32_t s_fps_window_ms;
static uint32_t s_fps_window_frames;
static uint16_t s_effective_fps_x10;

static void AppAcousticService_FillConfigSnapshot(AppAcousticServiceSnapshot_t *snapshot);

static uint8_t AppAcousticService_ClampPercentU32(uint32_t value)
{
  return (value > 100U) ? 100U : (uint8_t)value;
}

static float AppAcousticService_Clamp01(float value)
{
  if (value > 1.0f)
  {
    return 1.0f;
  }
  if (value < 0.0f)
  {
    return 0.0f;
  }
  return value;
}

static float AppAcousticService_AbsF32(float value)
{
  return (value < 0.0f) ? -value : value;
}

static int16_t AppAcousticService_RoundDeg(float value)
{
  if (value >= 0.0f)
  {
    return (int16_t)(value + 0.5f);
  }
  return (int16_t)(value - 0.5f);
}

static uint8_t AppAcousticService_PercentFromFloat(float value)
{
  value = AppAcousticService_Clamp01(value);
  return (uint8_t)((value * 100.0f) + 0.5f);
}

static uint16_t AppAcousticService_CyclesToMsX100(uint32_t cycles)
{
  if (SystemCoreClock == 0U)
  {
    return 0U;
  }

  return (uint16_t)(((uint64_t)cycles * 100000ULL) / (uint64_t)SystemCoreClock);
}

static uint16_t AppAcousticService_MsToX100(uint32_t elapsed_ms)
{
  uint32_t value = elapsed_ms * 100U;

  return (value > 65535U) ? 65535U : (uint16_t)value;
}

static uint8_t AppAcousticService_LoadFromElapsedMs(uint32_t elapsed_ms)
{
  uint32_t load;

  load = (uint32_t)(((uint64_t)elapsed_ms *
                    (uint64_t)APP_ACOUSTIC_SERVICE_TARGET_FPS *
                    100ULL) / 1000ULL);
  return AppAcousticService_ClampPercentU32(load);
}

static uint8_t AppAcousticService_RelativeCycleLoad(uint32_t cycles,
                                                    uint32_t total_cycles,
                                                    uint8_t total_load)
{
  if ((total_cycles == 0U) || (total_load == 0U))
  {
    return 0U;
  }

  return AppAcousticService_ClampPercentU32((uint32_t)(((uint64_t)cycles *
                                                       (uint64_t)total_load) /
                                                       (uint64_t)total_cycles));
}

static uint8_t AppAcousticService_ProfileIsValid(AppAcousticImagingProfile_t profile)
{
  return ((profile == APP_ACOUSTIC_IMAGING_PROFILE_FAST) ||
          (profile == APP_ACOUSTIC_IMAGING_PROFILE_BALANCED) ||
          (profile == APP_ACOUSTIC_IMAGING_PROFILE_QUALITY)) ? 1U : 0U;
}

static uint8_t AppAcousticService_ModeIsValid(AppAcousticImagingRunMode_t mode)
{
  return ((mode == APP_ACOUSTIC_IMAGING_MODE_FAST) ||
          (mode == APP_ACOUSTIC_IMAGING_MODE_STANDARD) ||
          (mode == APP_ACOUSTIC_IMAGING_MODE_HIGH_QUALITY)) ? 1U : 0U;
}

static AppAcousticImagingRunMode_t AppAcousticService_ModeFromProfile(AppAcousticImagingProfile_t profile)
{
  switch (profile)
  {
  case APP_ACOUSTIC_IMAGING_PROFILE_FAST:
    return APP_ACOUSTIC_IMAGING_MODE_FAST;
  case APP_ACOUSTIC_IMAGING_PROFILE_QUALITY:
    return APP_ACOUSTIC_IMAGING_MODE_HIGH_QUALITY;
  case APP_ACOUSTIC_IMAGING_PROFILE_BALANCED:
  default:
    return APP_ACOUSTIC_IMAGING_MODE_STANDARD;
  }
}

static AppAcousticImagingProfile_t AppAcousticService_ProfileFromMode(AppAcousticImagingRunMode_t mode)
{
  switch (mode)
  {
  case APP_ACOUSTIC_IMAGING_MODE_FAST:
    return APP_ACOUSTIC_IMAGING_PROFILE_FAST;
  case APP_ACOUSTIC_IMAGING_MODE_HIGH_QUALITY:
    return APP_ACOUSTIC_IMAGING_PROFILE_QUALITY;
  case APP_ACOUSTIC_IMAGING_MODE_STANDARD:
  default:
    return APP_ACOUSTIC_IMAGING_PROFILE_BALANCED;
  }
}

static uint8_t AppAcousticService_BinPolicyIsValid(AppAcousticImagingBinPolicy_t policy)
{
  return ((policy == APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT) ||
          (policy == APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B12) ||
          (policy == APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B16) ||
          (policy == APP_ACOUSTIC_IMAGING_BIN_POLICY_STANDARD_B24) ||
          (policy == APP_ACOUSTIC_IMAGING_BIN_POLICY_QUALITY_B40)) ? 1U : 0U;
}

static uint8_t AppAcousticService_TickReached(ULONG now, ULONG target)
{
  return (((LONG)(now - target)) >= 0) ? 1U : 0U;
}

static void AppAcousticService_PublishSnapshot(const AppAcousticServiceSnapshot_t *snapshot)
{
  uint32_t primask;

  if (snapshot == NULL)
  {
    return;
  }

  primask = __get_PRIMASK();
  __disable_irq();
  s_snapshot = *snapshot;
  if (primask == 0U)
  {
    __enable_irq();
  }
}

static void AppAcousticService_LoadSnapshot(AppAcousticServiceSnapshot_t *snapshot)
{
  uint32_t primask;

  if (snapshot == NULL)
  {
    return;
  }

  primask = __get_PRIMASK();
  __disable_irq();
  *snapshot = s_snapshot;
  if (primask == 0U)
  {
    __enable_irq();
  }
}

static AppAcousticImagingStatus_t AppAcousticService_InitRuntime(AppAcousticImagingRunMode_t mode,
                                                                 AppAcousticImagingBinPolicy_t bin_policy)
{
  AppAcousticImagingConfig_t config;
  AppAcousticImagingStatus_t status;
  const AppAcousticScene_t scene =
      (s_requested_scene < APP_ACOUSTIC_SCENE_COUNT) ? s_requested_scene : APP_ACOUSTIC_SCENE_GENERAL;
  const AppAcousticScenePreset_t *preset = &s_scene_presets[scene];
  const int8_t temperature_c = s_requested_temperature_c;

  status = App_AcousticImaging_GetDefaultRunModeConfig(mode, &config);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    memset(&s_srp_ctx, 0, sizeof(s_srp_ctx));
    return status;
  }

  config.ui_target_fps = APP_ACOUSTIC_SERVICE_TARGET_FPS;
  config.ui_min_fps = APP_ACOUSTIC_SERVICE_MIN_FPS;
  config.adaptive_profile_enable = 1U;

  status = App_AcousticImaging_SetBinPolicy(&config, bin_policy);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    memset(&s_srp_ctx, 0, sizeof(s_srp_ctx));
    return status;
  }

  /* Band priority: user's custom band (spectrum panel) > scene preset >
   * profile default bins. */
  {
    const uint16_t custom_lo = s_requested_band_lo_bin;
    const uint16_t custom_hi = s_requested_band_hi_bin;
    uint16_t band_lo = 0U;
    uint16_t band_hi = 0U;

    if (custom_lo != 0U)
    {
      band_lo = custom_lo;
      band_hi = custom_hi;
    }
    else if (preset->bin_lo != 0U)
    {
      band_lo = preset->bin_lo;
      band_hi = preset->bin_hi;
    }

    if (band_lo != 0U)
    {
      status = App_AcousticImaging_SetBand(&config, band_lo, band_hi);
      if (status != APP_ACOUSTIC_IMAGING_OK)
      {
        memset(&s_srp_ctx, 0, sizeof(s_srp_ctx));
        return status;
      }
    }
    s_active_band_lo_bin = custom_lo;
    s_active_band_hi_bin = custom_hi;
  }

  status = App_AcousticImaging_SetTemperature(&config, (float)temperature_c);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    memset(&s_srp_ctx, 0, sizeof(s_srp_ctx));
    return status;
  }

  status = App_AcousticSrp_Init(&s_srp_ctx, &config, APP_ACOUSTIC_BACKEND_F32_CMSIS);
  if (status == APP_ACOUSTIC_IMAGING_OK)
  {
    s_active_mode = mode;
    s_active_profile = config.profile;
    s_requested_profile = config.profile;
    s_active_scene = scene;
    s_active_temperature_c = temperature_c;
    s_over_budget_count = 0U;
    s_error_count = 0U;
    s_profile_change_pending = 0U;
    /* .EXTRAM state is not zero-initialised by startup; the EMA accumulator
     * is read before first write, so clear it here. */
    memset(s_field_accum, 0, sizeof(s_field_accum));
    s_field_peak_ema = 0.0f;
  }
  else
  {
    memset(&s_srp_ctx, 0, sizeof(s_srp_ctx));
  }

  return status;
}

static uint8_t AppAcousticService_CopyFrame(AppAudioFrame_t *dst, const AppAudioFrame_t *src)
{
  uint32_t sample_count;

  if ((dst == NULL) ||
      (src == NULL) ||
      (src->format != APP_AUDIO_FRAME_FORMAT_PLANAR_F32) ||
      (src->planar_f32 == NULL) ||
      (src->channel_count != APP_MIC_ARRAY_PHYSICAL_MIC_COUNT) ||
      (src->frame_len != APP_AUDIO_FRAME_DEFAULT_WIDE32_FRAME_LEN))
  {
    return 0U;
  }

  sample_count = src->channel_count * src->samples_per_channel;
  if ((sample_count == 0U) || (sample_count > APP_ACOUSTIC_SERVICE_SAMPLE_COUNT))
  {
    return 0U;
  }

  memcpy(s_service_samples, src->planar_f32, sample_count * sizeof(float));
  *dst = *src;
  dst->planar_f32 = s_service_samples;
  dst->planar_i16 = NULL;

  return 1U;
}

static void AppAcousticService_ClearHeat(AppAcousticServiceSnapshot_t *snapshot)
{
  if (snapshot != NULL)
  {
    memset(snapshot->field, 0, sizeof(snapshot->field));
    snapshot->cand_count = 0U;
  }
}

/* Map a field column/row centre to the SRP coarse-grid coordinate (u along
 * theta, v along phi, both in grid units of 15 deg). */
static float AppAcousticService_FieldColToTheta(uint32_t fx)
{
  return -APP_ACOUSTIC_SERVICE_CAMERA_HFOV_HALF_DEG +
         (((float)fx + 0.5f) * APP_ACOUSTIC_SERVICE_CAMERA_HFOV_DEG /
          (float)APP_ACOUSTIC_SERVICE_FIELD_W);
}

static float AppAcousticService_FieldRowToPhi(uint32_t fy)
{
  return APP_ACOUSTIC_SERVICE_CAMERA_VFOV_HALF_DEG -
         (((float)fy + 0.5f) * APP_ACOUSTIC_SERVICE_CAMERA_VFOV_DEG /
          (float)APP_ACOUSTIC_SERVICE_FIELD_H);
}

/* Catmull-Rom weight evaluation for the four taps around fractional t. */
static void AppAcousticService_CatmullRomWeights(float t, float w[4])
{
  const float t2 = t * t;
  const float t3 = t2 * t;

  w[0] = 0.5f * (-t3 + (2.0f * t2) - t);
  w[1] = 0.5f * ((3.0f * t3) - (5.0f * t2) + 2.0f);
  w[2] = 0.5f * ((-3.0f * t3) + (4.0f * t2) + t);
  w[3] = 0.5f * (t3 - t2);
}

/* Resample the regular 9x9 coarse SRP grid (theta-major layout, 15 deg
 * pitch, +/-60 deg) into the camera-aspect field with Catmull-Rom bicubic
 * sampling (C1-continuous - bilinear left visible diamond-shaped steps at
 * the 15 deg grid pitch), then fuse the fine points as Gaussian bumps. */
static void AppAcousticService_BuildLinearField(const AppAcousticImagingVisFrame_t *vis_frame)
{
  const float inv_step = 1.0f / 15.0f;
  const int32_t n = (int32_t)APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE;
  float coarse_abs[APP_ACOUSTIC_IMAGING_COARSE_TOTAL];
  /* Per-column tap indices/weights are identical for every row: precompute. */
  int32_t col_i0[APP_ACOUSTIC_SERVICE_FIELD_W];
  float col_w[APP_ACOUSTIC_SERVICE_FIELD_W][4];

  for (uint32_t i = 0U; i < APP_ACOUSTIC_IMAGING_COARSE_TOTAL; i++)
  {
    coarse_abs[i] = AppAcousticService_AbsF32(vis_frame->power[i]);
  }

  for (uint32_t fx = 0U; fx < APP_ACOUSTIC_SERVICE_FIELD_W; fx++)
  {
    const float theta = AppAcousticService_FieldColToTheta(fx);
    float u = (theta + 60.0f) * inv_step;

    if (u < 0.0f)
    {
      u = 0.0f;
    }
    if (u > (float)(n - 1))
    {
      u = (float)(n - 1);
    }
    col_i0[fx] = (int32_t)u;
    if (col_i0[fx] > (n - 2))
    {
      col_i0[fx] = n - 2;
    }
    AppAcousticService_CatmullRomWeights(u - (float)col_i0[fx], col_w[fx]);
  }

  for (uint32_t fy = 0U; fy < APP_ACOUSTIC_SERVICE_FIELD_H; fy++)
  {
    const float phi = AppAcousticService_FieldRowToPhi(fy);
    float v = (phi + 60.0f) * inv_step;
    int32_t v0;
    float wv[4];
    float row_taps[APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE];

    if (v < 0.0f)
    {
      v = 0.0f;
    }
    if (v > (float)(n - 1))
    {
      v = (float)(n - 1);
    }
    v0 = (int32_t)v;
    if (v0 > (n - 2))
    {
      v0 = n - 2;
    }
    AppAcousticService_CatmullRomWeights(v - (float)v0, wv);

    /* Vertical pass: blend the four phi taps (edge-clamped) for each of the
     * nine theta columns. Layout: index = theta_idx * n + phi_idx. */
    for (int32_t tx = 0; tx < n; tx++)
    {
      float acc = 0.0f;

      for (int32_t k = 0; k < 4; k++)
      {
        int32_t py = v0 - 1 + k;

        if (py < 0)
        {
          py = 0;
        }
        if (py > (n - 1))
        {
          py = n - 1;
        }
        acc += wv[k] * coarse_abs[(tx * n) + py];
      }
      row_taps[tx] = acc;
    }

    for (uint32_t fx = 0U; fx < APP_ACOUSTIC_SERVICE_FIELD_W; fx++)
    {
      const int32_t u0 = col_i0[fx];
      const float *wu = col_w[fx];
      float sample = 0.0f;

      for (int32_t k = 0; k < 4; k++)
      {
        int32_t px = u0 - 1 + k;

        if (px < 0)
        {
          px = 0;
        }
        if (px > (n - 1))
        {
          px = n - 1;
        }
        sample += wu[k] * row_taps[px];
      }

      /* Catmull-Rom overshoots near sharp peaks; heat power is
       * non-negative by construction. */
      if (sample < 0.0f)
      {
        sample = 0.0f;
      }
      s_field_work[(fy * APP_ACOUSTIC_SERVICE_FIELD_W) + fx] = sample;
    }
  }

  /* Fine points: Gaussian bumps around the refined peaks. */
  if (vis_frame->grid_count > APP_ACOUSTIC_IMAGING_COARSE_TOTAL)
  {
    const float sigma = APP_ACOUSTIC_SERVICE_FIELD_FINE_SIGMA;
    const float inv_two_sigma_sq = 1.0f / (2.0f * sigma * sigma);
    const int32_t radius = 8;
    const float fx_scale = (float)APP_ACOUSTIC_SERVICE_FIELD_W / APP_ACOUSTIC_SERVICE_CAMERA_HFOV_DEG;
    const float fy_scale = (float)APP_ACOUSTIC_SERVICE_FIELD_H / APP_ACOUSTIC_SERVICE_CAMERA_VFOV_DEG;

    for (uint32_t i = APP_ACOUSTIC_IMAGING_COARSE_TOTAL; i < vis_frame->grid_count; i++)
    {
      const float theta = vis_frame->theta_deg[i];
      const float phi = vis_frame->phi_deg[i];
      const float value = AppAcousticService_AbsF32(vis_frame->power[i]) *
                          APP_ACOUSTIC_SERVICE_FIELD_FINE_GAIN;
      float gx;
      float gy;
      int32_t cx;
      int32_t cy;

      if ((value <= 0.0f) ||
          (theta < -APP_ACOUSTIC_SERVICE_CAMERA_HFOV_HALF_DEG) ||
          (theta > APP_ACOUSTIC_SERVICE_CAMERA_HFOV_HALF_DEG) ||
          (phi < -APP_ACOUSTIC_SERVICE_CAMERA_VFOV_HALF_DEG) ||
          (phi > APP_ACOUSTIC_SERVICE_CAMERA_VFOV_HALF_DEG))
      {
        continue;
      }

      gx = (theta + APP_ACOUSTIC_SERVICE_CAMERA_HFOV_HALF_DEG) * fx_scale - 0.5f;
      gy = (APP_ACOUSTIC_SERVICE_CAMERA_VFOV_HALF_DEG - phi) * fy_scale - 0.5f;
      cx = (int32_t)(gx + 0.5f);
      cy = (int32_t)(gy + 0.5f);

      for (int32_t dy = -radius; dy <= radius; dy++)
      {
        const int32_t yy = cy + dy;
        if ((yy < 0) || (yy >= (int32_t)APP_ACOUSTIC_SERVICE_FIELD_H))
        {
          continue;
        }
        for (int32_t dx = -radius; dx <= radius; dx++)
        {
          const int32_t xx = cx + dx;
          float ddx;
          float ddy;
          float dist_sq;
          float bump;

          if ((xx < 0) || (xx >= (int32_t)APP_ACOUSTIC_SERVICE_FIELD_W))
          {
            continue;
          }

          ddx = (float)xx - gx;
          ddy = (float)yy - gy;
          dist_sq = (ddx * ddx) + (ddy * ddy);
          bump = value * expf(-dist_sq * inv_two_sigma_sq);
          s_field_work[(yy * (int32_t)APP_ACOUSTIC_SERVICE_FIELD_W) + xx] += bump;
        }
      }
    }
  }
}

/* Load the runtime field parameters for this frame (UI thread writes them
 * with IRQs masked; a masked copy here gives a coherent snapshot). */
static void AppAcousticService_LoadFieldParams(void)
{
  uint32_t primask = __get_PRIMASK();

  __disable_irq();
  s_field_active = s_field_params;
  if (primask == 0U)
  {
    __enable_irq();
  }
}

/* One separable 3x3 [1 2 1] smoothing pass over the linear field. */
static void AppAcousticService_SmoothField(void)
{
  const uint32_t w = APP_ACOUSTIC_SERVICE_FIELD_W;
  const uint32_t h = APP_ACOUSTIC_SERVICE_FIELD_H;

  for (uint32_t y = 0U; y < h; y++)
  {
    const float *row = &s_field_work[y * w];
    float *dst = &s_field_smooth[y * w];

    dst[0] = ((3.0f * row[0]) + row[1]) * 0.25f;
    for (uint32_t x = 1U; x < (w - 1U); x++)
    {
      dst[x] = (row[x - 1U] + (2.0f * row[x]) + row[x + 1U]) * 0.25f;
    }
    dst[w - 1U] = (row[w - 2U] + (3.0f * row[w - 1U])) * 0.25f;
  }

  for (uint32_t x = 0U; x < w; x++)
  {
    float previous = s_field_smooth[x];

    s_field_work[x] = ((3.0f * s_field_smooth[x]) + s_field_smooth[w + x]) * 0.25f;
    for (uint32_t y = 1U; y < (h - 1U); y++)
    {
      const float current = s_field_smooth[(y * w) + x];
      const float next = s_field_smooth[((y + 1U) * w) + x];
      s_field_work[(y * w) + x] = (previous + (2.0f * current) + next) * 0.25f;
      previous = current;
    }
    s_field_work[((h - 1U) * w) + x] =
        (previous + (3.0f * s_field_smooth[((h - 1U) * w) + x])) * 0.25f;
  }
}

/* Temporal EMA, adaptive background subtraction, dB windowing, gamma, then
 * quantisation to 0..255. */
static void AppAcousticService_NormalizeField(AppAcousticServiceSnapshot_t *snapshot)
{
  float peak = 0.0f;
  float mean = 0.0f;
  float floor_lin;
  float bg_floor;
  float span_db = -s_field_active.db_floor;
  float inv_span_db = 1.0f / span_db;

  for (uint32_t i = 0U; i < APP_ACOUSTIC_SERVICE_FIELD_COUNT; i++)
  {
    const float attack = APP_ACOUSTIC_SERVICE_FIELD_EMA_ATTACK;
    const float decay = APP_ACOUSTIC_SERVICE_FIELD_EMA_DECAY;
    const float value = s_field_work[i];
    const float accum = s_field_accum[i];
    const float alpha = (value > accum) ? attack : decay;

    s_field_accum[i] = accum + (alpha * (value - accum));
    if (s_field_accum[i] > peak)
    {
      peak = s_field_accum[i];
    }
    mean += s_field_accum[i];
  }
  mean *= (1.0f / (float)APP_ACOUSTIC_SERVICE_FIELD_COUNT);

  if (peak > s_field_peak_ema)
  {
    s_field_peak_ema += APP_ACOUSTIC_SERVICE_FIELD_EMA_ATTACK * (peak - s_field_peak_ema);
  }
  else
  {
    s_field_peak_ema += APP_ACOUSTIC_SERVICE_FIELD_EMA_DECAY * (peak - s_field_peak_ema);
  }

  if (s_field_peak_ema < 1.0e-12f)
  {
    memset(snapshot->field, 0, sizeof(snapshot->field));
    return;
  }

  floor_lin = s_field_active.noise_gate * s_field_peak_ema;
  bg_floor = mean * s_field_active.bg_gain;
  if (bg_floor > floor_lin)
  {
    floor_lin = bg_floor;
  }

  {
  const float peak_effective = s_field_peak_ema - floor_lin;

  if (peak_effective < 1.0e-12f)
  {
    memset(snapshot->field, 0, sizeof(snapshot->field));
    return;
  }

  for (uint32_t i = 0U; i < APP_ACOUSTIC_SERVICE_FIELD_COUNT; i++)
  {
    float effective = s_field_accum[i] - floor_lin;
    float norm;
    float db;

    if (effective <= 0.0f)
    {
      snapshot->field[i] = 0U;
      continue;
    }

    norm = effective / peak_effective;
    if (norm > 1.0f)
    {
      norm = 1.0f;
    }

    /* 20*log10(norm) mapped from [db_floor, 0] onto [0, 1]. */
    db = 20.0f * log10f(norm);
    if (db <= s_field_active.db_floor)
    {
      snapshot->field[i] = 0U;
      continue;
    }

    norm = (db + span_db) * inv_span_db;
    norm = powf(norm, s_field_active.gamma);
    snapshot->field[i] = (uint8_t)((norm * 255.0f) + 0.5f);
  }
  }
}

static void AppAcousticService_FillCameraField(AppAcousticServiceSnapshot_t *snapshot,
                                               const AppAcousticImagingVisFrame_t *vis_frame)
{
  float peak_abs;

  if ((snapshot == NULL) || (vis_frame == NULL))
  {
    return;
  }

  peak_abs = AppAcousticService_AbsF32(vis_frame->peak_value);
  if (peak_abs < 1.0e-9f)
  {
    AppAcousticService_ClearHeat(snapshot);
    return;
  }

  AppAcousticService_LoadFieldParams();
  AppAcousticService_BuildLinearField(vis_frame);
  for (uint32_t pass = 0U; pass < s_field_active.smooth_passes; pass++)
  {
    AppAcousticService_SmoothField();
  }
  AppAcousticService_NormalizeField(snapshot);

  /* Strength as prominence above the field floor, not raw power ratio: the
   * smoothed SRP surface keeps every grid point at 85-98% of the peak, so
   * power ratios read like fake confidences (S2/S3 pinned at ~95%).
   * Min-subtraction restores discrimination between echoes and sidelobes. */
  {
    float floor_abs = peak_abs;
    float denom;

    for (uint32_t i = 0U; i < vis_frame->grid_count; i++)
    {
      const float p = AppAcousticService_AbsF32(vis_frame->power[i]);
      if (p < floor_abs)
      {
        floor_abs = p;
      }
    }
    denom = peak_abs - floor_abs;
    if (denom < 1.0e-9f)
    {
      denom = 1.0e-9f;
    }

    snapshot->cand_count = 0U;
    for (uint32_t i = 0U; (i < vis_frame->candidate_count) && (i < APP_ACOUSTIC_SERVICE_CAND_MAX); i++)
    {
      const AppAcousticImagingCandidate_t *cand = &vis_frame->candidate[i];
      float strength = (AppAcousticService_AbsF32(cand->power) - floor_abs) / denom;

      snapshot->cand_theta[i] = AppAcousticService_RoundDeg(cand->theta_deg);
      snapshot->cand_phi[i] = AppAcousticService_RoundDeg(cand->phi_deg);
      snapshot->cand_strength[i] = (uint8_t)((AppAcousticService_Clamp01(strength) * 255.0f) + 0.5f);
      snapshot->cand_count++;
    }
  }
}

static void AppAcousticService_UpdatePerf(AppAcousticServiceSnapshot_t *snapshot,
                                          const AppAcousticSrpPerf_t *perf,
                                          uint32_t elapsed_ms)
{
  uint32_t srp_cycles;
  uint8_t total_load;

  if ((snapshot == NULL) || (perf == NULL))
  {
    return;
  }

  srp_cycles = perf->coarse_cycles + perf->fine_cycles + perf->output_cycles;
  total_load = AppAcousticService_LoadFromElapsedMs(elapsed_ms);

  snapshot->perf = *perf;
  snapshot->srp_ms_x100 = (elapsed_ms != 0U) ?
                          AppAcousticService_MsToX100(elapsed_ms) :
                          AppAcousticService_CyclesToMsX100(perf->total_cycles);
  snapshot->perf_load[0] = AppAcousticService_RelativeCycleLoad(perf->preprocess_cycles,
                                                                perf->total_cycles,
                                                                total_load);
  snapshot->perf_load[1] = AppAcousticService_RelativeCycleLoad(perf->fft_cycles,
                                                                perf->total_cycles,
                                                                total_load);
  snapshot->perf_load[2] = AppAcousticService_RelativeCycleLoad(perf->gcc_cycles,
                                                                perf->total_cycles,
                                                                total_load);
  snapshot->perf_load[3] = AppAcousticService_RelativeCycleLoad(srp_cycles,
                                                                perf->total_cycles,
                                                                total_load);
  snapshot->perf_load[4] = total_load;
}

/* Log-scaled display spectrum from the SRP reference-channel FFT.
 * Peak tracking uses a slow-decay EMA so the bars stay readable across
 * loud/quiet scenes without manual gain. */
static void AppAcousticService_FillSpectrum(AppAcousticServiceSnapshot_t *snapshot)
{
  static float s_spectrum_peak_ema;
  float mags[APP_ACOUSTIC_SERVICE_SPECTRUM_BINS + 1U];
  float peak = 0.0f;
  uint32_t got;
  uint32_t peak_bin = 0U;

  got = App_AcousticSrp_GetRefSpectrum(&s_srp_ctx, mags, APP_ACOUSTIC_SERVICE_SPECTRUM_BINS + 1U);
  if (got <= 1U)
  {
    memset(snapshot->spectrum, 0, sizeof(snapshot->spectrum));
    snapshot->spectrum_peak_bin = 0U;
    return;
  }

  for (uint32_t bin = 1U; bin < got; bin++)
  {
    if (mags[bin] > peak)
    {
      peak = mags[bin];
      peak_bin = bin;
    }
  }

  if (peak > s_spectrum_peak_ema)
  {
    s_spectrum_peak_ema += 0.5f * (peak - s_spectrum_peak_ema);
  }
  else
  {
    s_spectrum_peak_ema += 0.02f * (peak - s_spectrum_peak_ema);
  }
  if (s_spectrum_peak_ema < 1.0e-9f)
  {
    memset(snapshot->spectrum, 0, sizeof(snapshot->spectrum));
    snapshot->spectrum_peak_bin = 0U;
    return;
  }

  for (uint32_t i = 0U; i < APP_ACOUSTIC_SERVICE_SPECTRUM_BINS; i++)
  {
    const uint32_t bin = i + 1U;
    float norm = ((bin < got) ? mags[bin] : 0.0f) / s_spectrum_peak_ema;
    float db;
    float level;

    if (norm <= 0.001f)
    {
      snapshot->spectrum[i] = 0U;
      continue;
    }
    if (norm > 1.0f)
    {
      norm = 1.0f;
    }

    /* 60 dB display window. */
    db = 20.0f * log10f(norm);
    level = (db + 60.0f) * (1.0f / 60.0f);
    if (level < 0.0f)
    {
      level = 0.0f;
    }
    snapshot->spectrum[i] = (uint8_t)((level * 255.0f) + 0.5f);
  }

  snapshot->spectrum_peak_bin = (uint8_t)((peak_bin <= 255U) ? peak_bin : 255U);

  /* Feed the Neural-ART sound classifier with the same display spectrum
   * (rolling 32-frame window inside the NPU service). */
  AppNpu_FeedSpectrum(snapshot->spectrum);
}

static void AppAcousticService_UpdateFps(AppAcousticServiceSnapshot_t *snapshot)
{
  uint32_t now_ms = HAL_GetTick();
  uint32_t elapsed_ms;

  if (s_fps_window_ms == 0U)
  {
    s_fps_window_ms = now_ms;
    s_fps_window_frames = 0U;
  }

  s_fps_window_frames++;
  elapsed_ms = now_ms - s_fps_window_ms;
  if (elapsed_ms >= 1000U)
  {
    s_effective_fps_x10 = (uint16_t)(((uint64_t)s_fps_window_frames * 10000ULL) /
                                     (uint64_t)elapsed_ms);
    s_fps_window_ms = now_ms;
    s_fps_window_frames = 0U;
  }

  snapshot->fps_x10 = s_effective_fps_x10;
}

static void AppAcousticService_FillVisSnapshot(AppAcousticServiceSnapshot_t *snapshot,
                                               const AppAcousticImagingVisFrame_t *vis_frame,
                                               const AppAcousticSrpContext_t *ctx)
{
  const AppAcousticImagingCandidate_t *candidate;

  if ((snapshot == NULL) || (vis_frame == NULL) || (ctx == NULL))
  {
    return;
  }

  candidate = (vis_frame->candidate_count > 0U) ? &vis_frame->candidate[0] : NULL;
  snapshot->valid = vis_frame->valid;
  snapshot->output_seq = vis_frame->frame_seq;
  snapshot->active_mode = ctx->config.run_mode;
  snapshot->active_profile = ctx->config.profile;
  snapshot->active_bin_policy = ctx->config.bin_policy;
  snapshot->backend = ctx->backend;
  snapshot->active_channel_mask = ctx->active_channel_mask;
  snapshot->pair_count = ctx->pair_count;
  snapshot->grid_count = ctx->grid_count;
  snapshot->active_bin_count = (uint16_t)ctx->active_bin_count;
  snapshot->quality_pct = AppAcousticService_PercentFromFloat(vis_frame->quality);
  snapshot->contrast_pct = AppAcousticService_PercentFromFloat(vis_frame->contrast);
  if (candidate != NULL)
  {
    snapshot->theta_deg = AppAcousticService_RoundDeg(candidate->theta_deg);
    snapshot->phi_deg = AppAcousticService_RoundDeg(candidate->phi_deg);
  }
  else
  {
    snapshot->theta_deg = 0;
    snapshot->phi_deg = 0;
  }
  AppAcousticService_FillCameraField(snapshot, vis_frame);
}

static void AppAcousticService_UpdateDegrade(AppAcousticServiceSnapshot_t *snapshot,
                                             AppAcousticImagingStatus_t status,
                                             uint32_t elapsed_ms)
{
  uint32_t min_budget_ms;

  if (snapshot == NULL)
  {
    return;
  }

  if (g_app_acoustic_service_disable_auto_degrade != 0U)
  {
    s_over_budget_count = 0U;
    s_error_count = 0U;
    return;
  }

  min_budget_ms = 1000U / APP_ACOUSTIC_SERVICE_MIN_FPS;

  if (status == APP_ACOUSTIC_IMAGING_OK)
  {
    s_error_count = 0U;
    if ((elapsed_ms != 0U) && (elapsed_ms > min_budget_ms))
    {
      s_over_budget_count++;
    }
    else
    {
      s_over_budget_count = 0U;
    }
  }
  else
  {
    s_error_count++;
  }

  if ((s_active_mode != APP_ACOUSTIC_IMAGING_MODE_FAST) &&
      ((s_over_budget_count >= APP_ACOUSTIC_SERVICE_DEGRADE_LIMIT) ||
       (s_error_count >= APP_ACOUSTIC_SERVICE_DEGRADE_LIMIT)))
  {
    s_requested_mode = APP_ACOUSTIC_IMAGING_MODE_FAST;
    s_requested_profile = APP_ACOUSTIC_IMAGING_PROFILE_FAST;
    s_requested_bin_policy = APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT;
    s_profile_change_pending = 1U;
    snapshot->auto_degraded = 1U;
    snapshot->degraded_count++;
  }
}

AppAcousticImagingStatus_t AppAcousticService_Init(void)
{
  AppAcousticImagingStatus_t status;
  AppAcousticImagingRunMode_t mode = s_requested_mode;

  if (AppAcousticService_ModeIsValid(mode) == 0U)
  {
    mode = APP_ACOUSTIC_IMAGING_MODE_STANDARD;
    s_requested_mode = mode;
  }
  if (AppAcousticService_BinPolicyIsValid(s_requested_bin_policy) == 0U)
  {
    s_requested_bin_policy = APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT;
  }

  status = AppAcousticService_InitRuntime(mode, s_requested_bin_policy);

  /* Runs from tx_application_define before any thread starts, so writing the
   * shared snapshot directly is race-free. The snapshot struct is ~1.9 KB;
   * keeping it off the (limit-checked) init stack avoids an MSPLIM overflow. */
  memset((void *)&s_snapshot, 0, sizeof(s_snapshot));
  s_snapshot.requested_mode = s_requested_mode;
  s_snapshot.requested_profile = s_requested_profile;
  s_snapshot.requested_bin_policy = s_requested_bin_policy;
  s_snapshot.backend = APP_ACOUSTIC_BACKEND_F32_CMSIS;
  s_snapshot.service_status = APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME;
  s_snapshot.initialized = (status == APP_ACOUSTIC_IMAGING_OK) ? 1U : 0U;
  s_snapshot.last_status = (int32_t)status;
  s_snapshot.active_mode = s_active_mode;
  s_snapshot.active_profile = s_active_profile;
  s_snapshot.active_bin_policy = s_srp_ctx.config.bin_policy;
  s_snapshot.active_channel_mask = s_srp_ctx.active_channel_mask;
  s_snapshot.pair_count = s_srp_ctx.pair_count;
  s_snapshot.grid_count = s_srp_ctx.grid_count;
  s_snapshot.active_bin_count = (uint16_t)s_srp_ctx.active_bin_count;
  AppAcousticService_FillConfigSnapshot(&s_snapshot);
  __DMB();

  return status;
}

/* Mirror scene/temperature/band/params into the published snapshot. */
static void AppAcousticService_FillConfigSnapshot(AppAcousticServiceSnapshot_t *snapshot)
{
  snapshot->scene = (uint8_t)s_active_scene;
  snapshot->temperature_c = s_active_temperature_c;
  snapshot->speed_mps_x10 = (uint16_t)((s_srp_ctx.config.speed_of_sound_mps * 10.0f) + 0.5f);
  snapshot->band_lo_hz =
      (uint16_t)(((float)s_srp_ctx.config.active_bin_start * APP_ACOUSTIC_SERVICE_BIN_HZ) + 0.5f);
  snapshot->band_hi_hz =
      (uint16_t)(((float)s_srp_ctx.config.active_bin_end * APP_ACOUSTIC_SERVICE_BIN_HZ) + 0.5f);
  AppAcousticService_GetFieldParams(&snapshot->field_params);
}

/* Sanitize the requested mode/policy and reinitialize the SRP runtime when a
 * change is pending. Returns 1 when the runtime is ready for processing. */
static uint8_t AppAcousticService_SyncRuntimeConfig(AppAcousticServiceSnapshot_t *snapshot)
{
  AppAcousticImagingStatus_t status;

  if (AppAcousticService_ModeIsValid(s_requested_mode) == 0U)
  {
    s_requested_mode = APP_ACOUSTIC_IMAGING_MODE_STANDARD;
    s_profile_change_pending = 1U;
  }
  s_requested_profile = AppAcousticService_ProfileFromMode(s_requested_mode);
  if (AppAcousticService_BinPolicyIsValid(s_requested_bin_policy) == 0U)
  {
    s_requested_bin_policy = APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT;
    s_profile_change_pending = 1U;
  }
  snapshot->requested_mode = s_requested_mode;
  snapshot->requested_profile = s_requested_profile;
  snapshot->requested_bin_policy = s_requested_bin_policy;

  if ((s_profile_change_pending == 0U) &&
      (s_srp_ctx.initialized != 0U) &&
      (s_active_mode == s_requested_mode) &&
      (s_active_scene == s_requested_scene) &&
      (s_active_temperature_c == s_requested_temperature_c) &&
      (s_active_band_lo_bin == s_requested_band_lo_bin) &&
      (s_active_band_hi_bin == s_requested_band_hi_bin) &&
      (s_srp_ctx.config.bin_policy ==
       App_AcousticImaging_ResolveBinPolicy(s_requested_profile, s_requested_bin_policy)))
  {
    return 1U;
  }

  status = AppAcousticService_InitRuntime(s_requested_mode, s_requested_bin_policy);
  snapshot->initialized = (status == APP_ACOUSTIC_IMAGING_OK) ? 1U : 0U;
  snapshot->last_status = (int32_t)status;
  snapshot->service_status = (status == APP_ACOUSTIC_IMAGING_OK) ?
                             APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME :
                             (int32_t)status;
  snapshot->active_mode = s_active_mode;
  snapshot->active_profile = s_active_profile;
  snapshot->active_bin_policy = s_srp_ctx.config.bin_policy;
  snapshot->active_channel_mask = s_srp_ctx.active_channel_mask;
  snapshot->pair_count = s_srp_ctx.pair_count;
  snapshot->grid_count = s_srp_ctx.grid_count;
  snapshot->active_bin_count = (uint16_t)s_srp_ctx.active_bin_count;
  AppAcousticService_FillConfigSnapshot(snapshot);
  AppAcousticService_PublishSnapshot(snapshot);

  return (status == APP_ACOUSTIC_IMAGING_OK) ? 1U : 0U;
}

/* Publish a skip/failure snapshot and report it to the bring-up tracker. */
static void AppAcousticService_PublishSkip(AppAcousticServiceSnapshot_t *snapshot,
                                           int32_t service_status,
                                           uint8_t clear_result,
                                           uint8_t is_failure)
{
  if (is_failure != 0U)
  {
    snapshot->failed_frames++;
  }
  else
  {
    snapshot->skipped_frames++;
  }
  if (clear_result != 0U)
  {
    snapshot->valid = 0U;
    AppAcousticService_ClearHeat(snapshot);
  }
  snapshot->service_status = service_status;
  AppAcousticService_PublishSnapshot(snapshot);
  if (is_failure != 0U)
  {
    App_BringUpStatus_Fail(APP_BRINGUP_MODULE_ACOUSTIC, service_status);
  }
  else
  {
    App_BringUpStatus_Heartbeat(APP_BRINGUP_MODULE_ACOUSTIC, service_status);
  }
}

/* Run SRP on one frame and fill the snapshot with the outcome. */
static AppAcousticImagingStatus_t AppAcousticService_ProcessFrame(AppAcousticServiceSnapshot_t *snapshot,
                                                                  const AppAudioFrame_t *work_frame,
                                                                  uint32_t frame_seq,
                                                                  uint32_t *elapsed_ms)
{
  AppAcousticImagingStatus_t status;
  uint32_t process_start_ms = HAL_GetTick();

  status = App_AcousticSrp_ProcessFrame(&s_srp_ctx, work_frame, &s_vis_frame);
  *elapsed_ms = HAL_GetTick() - process_start_ms;

  snapshot->last_status = (int32_t)status;
  if (status == APP_ACOUSTIC_IMAGING_OK)
  {
    snapshot->service_status = APP_ACOUSTIC_SERVICE_STATUS_OK;
    snapshot->processed_frames++;
    s_have_input_seq = 1U;
    s_last_input_seq = frame_seq;
    AppAcousticService_FillVisSnapshot(snapshot, &s_vis_frame, &s_srp_ctx);
    AppAcousticService_FillSpectrum(snapshot);
    App_AcousticSrp_GetPerf(&s_srp_ctx, &snapshot->perf);
    AppAcousticService_UpdatePerf(snapshot, &snapshot->perf, *elapsed_ms);
    AppAcousticService_UpdateFps(snapshot);
    App_BringUpStatus_Ready(APP_BRINGUP_MODULE_ACOUSTIC, (int32_t)status);
  }
  else
  {
    snapshot->failed_frames++;
    snapshot->valid = 0U;
    snapshot->service_status = (int32_t)status;
    AppAcousticService_ClearHeat(snapshot);
    App_BringUpStatus_Fail(APP_BRINGUP_MODULE_ACOUSTIC, (int32_t)status);
  }

  return status;
}

void AppAcousticService_ThreadEntry(ULONG thread_input)
{
  AppAudioFrame_t capture_frame;
  AppAudioFrame_t work_frame;
  /* ~1.9 KB with the heat field; static (single service thread) instead of
   * eating a sixth of the thread stack. Fully written by LoadSnapshot before
   * first read, so uninitialised external RAM is fine. */
  static AppAcousticServiceSnapshot_t snapshot
      __attribute__((section(".EXTRAM"), aligned(32)));
  ULONG next_process_tick;
  ULONG period_ticks = APP_ACOUSTIC_SERVICE_PROCESS_PERIOD_TICKS;
  uint32_t process_elapsed_ms = 0U;

  (void)thread_input;
  if (period_ticks == 0U)
  {
    period_ticks = 1U;
  }

  AppAcousticService_LoadSnapshot(&snapshot);
  snapshot.running = 1U;
  AppAcousticService_PublishSnapshot(&snapshot);
  next_process_tick = tx_time_get();

  while (1)
  {
    AppAcousticImagingStatus_t status;
    ULONG now_tick = tx_time_get();

    if (AppAcousticService_TickReached(now_tick, next_process_tick) == 0U)
    {
      tx_thread_sleep(1U);
      continue;
    }
    next_process_tick = now_tick + period_ticks;

    /* Phase 1: pick up mode/profile/bin-policy changes. */
    AppAcousticService_LoadSnapshot(&snapshot);
    snapshot.running = 1U;
    if (AppAcousticService_SyncRuntimeConfig(&snapshot) == 0U)
    {
      tx_thread_sleep(1U);
      continue;
    }

    /* Phase 2: acquire a fresh PCMD frame. */
    if (AppPcmdCapture_GetLatestAudioFrame(&capture_frame) == 0U)
    {
      AppAcousticService_PublishSkip(&snapshot,
                                     APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME,
                                     1U, 0U);
      continue;
    }

    if ((s_have_input_seq != 0U) && (capture_frame.seq == s_last_input_seq))
    {
      snapshot.input_seq = capture_frame.seq;
      AppAcousticService_PublishSkip(&snapshot,
                                     APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME,
                                     0U, 0U);
      continue;
    }

    snapshot.input_seq = capture_frame.seq;
    if (AppAcousticService_CopyFrame(&work_frame, &capture_frame) == 0U)
    {
      AppAcousticService_PublishSkip(&snapshot,
                                     APP_ACOUSTIC_SERVICE_STATUS_COPY_FAILED,
                                     1U, 1U);
      continue;
    }

    /* Phase 3: process and publish. */
    status = AppAcousticService_ProcessFrame(&snapshot, &work_frame,
                                             capture_frame.seq,
                                             &process_elapsed_ms);
    AppAcousticService_UpdateDegrade(&snapshot, status, process_elapsed_ms);
    AppAcousticService_PublishSnapshot(&snapshot);
  }
}

void AppAcousticService_GetSnapshot(AppAcousticServiceSnapshot_t *snapshot)
{
  AppAcousticService_LoadSnapshot(snapshot);
}

AppAcousticImagingStatus_t AppAcousticService_SetProfile(AppAcousticImagingProfile_t profile)
{
  if (AppAcousticService_ProfileIsValid(profile) == 0U)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  s_requested_mode = AppAcousticService_ModeFromProfile(profile);
  s_requested_profile = profile;
  s_requested_bin_policy = APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT;
  s_profile_change_pending = 1U;

  return APP_ACOUSTIC_IMAGING_OK;
}

AppAcousticImagingStatus_t AppAcousticService_SetMode(AppAcousticImagingRunMode_t mode)
{
  if (AppAcousticService_ModeIsValid(mode) == 0U)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  s_requested_mode = mode;
  s_requested_profile = AppAcousticService_ProfileFromMode(mode);
  s_requested_bin_policy = APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT;
  s_profile_change_pending = 1U;

  return APP_ACOUSTIC_IMAGING_OK;
}

AppAcousticImagingStatus_t AppAcousticService_SetScene(AppAcousticScene_t scene)
{
  if (scene >= APP_ACOUSTIC_SCENE_COUNT)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  /* Scene switch re-baselines the rendering parameters to the preset and
   * drops any custom band from the spectrum panel. */
  AppAcousticService_SetFieldParams(&s_scene_presets[scene].params);
  s_requested_band_lo_bin = 0U;
  s_requested_band_hi_bin = 0U;
  s_requested_scene = scene;
  s_profile_change_pending = 1U;

  return APP_ACOUSTIC_IMAGING_OK;
}

AppAcousticImagingStatus_t AppAcousticService_SetBandHz(uint16_t lo_hz, uint16_t hi_hz)
{
  /* Observable range of the Wide32/48k pipeline: bins 3..42. */
  const uint16_t bin_min = 3U;
  const uint16_t bin_max = 42U;
  uint16_t lo_bin;
  uint16_t hi_bin;

  if (lo_hz >= hi_hz)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  lo_bin = (uint16_t)(((float)lo_hz / APP_ACOUSTIC_SERVICE_BIN_HZ) + 0.5f);
  hi_bin = (uint16_t)(((float)hi_hz / APP_ACOUSTIC_SERVICE_BIN_HZ) + 0.5f);
  if (lo_bin < bin_min)
  {
    lo_bin = bin_min;
  }
  if (hi_bin > bin_max)
  {
    hi_bin = bin_max;
  }
  /* Keep a usable window: at least 4 bins (~750 Hz). */
  if ((hi_bin <= lo_bin) || ((uint16_t)(hi_bin - lo_bin) < 3U))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  s_requested_band_lo_bin = lo_bin;
  s_requested_band_hi_bin = hi_bin;
  s_profile_change_pending = 1U;

  return APP_ACOUSTIC_IMAGING_OK;
}

AppAcousticImagingStatus_t AppAcousticService_SetTemperature(int8_t temperature_c)
{
  if ((temperature_c < APP_ACOUSTIC_SERVICE_TEMP_MIN_C) ||
      (temperature_c > APP_ACOUSTIC_SERVICE_TEMP_MAX_C))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  s_requested_temperature_c = temperature_c;
  s_profile_change_pending = 1U;

  return APP_ACOUSTIC_IMAGING_OK;
}

static float AppAcousticService_ClampF32(float value, float lo, float hi)
{
  if (value < lo)
  {
    return lo;
  }
  if (value > hi)
  {
    return hi;
  }
  return value;
}

void AppAcousticService_SetFieldParams(const AppAcousticFieldParams_t *params)
{
  AppAcousticFieldParams_t next;
  uint32_t primask;

  if (params == NULL)
  {
    return;
  }

  next.db_floor = AppAcousticService_ClampF32(params->db_floor, -30.0f, -6.0f);
  next.gamma = AppAcousticService_ClampF32(params->gamma, 0.5f, 2.5f);
  next.noise_gate = AppAcousticService_ClampF32(params->noise_gate, 0.0f, 0.6f);
  next.bg_gain = AppAcousticService_ClampF32(params->bg_gain, 1.0f, 3.0f);
  next.smooth_passes = (params->smooth_passes > 3U) ? 3U : params->smooth_passes;

  primask = __get_PRIMASK();
  __disable_irq();
  s_field_params = next;
  if (primask == 0U)
  {
    __enable_irq();
  }
}

void AppAcousticService_GetFieldParams(AppAcousticFieldParams_t *params)
{
  uint32_t primask;

  if (params == NULL)
  {
    return;
  }

  primask = __get_PRIMASK();
  __disable_irq();
  *params = s_field_params;
  if (primask == 0U)
  {
    __enable_irq();
  }
}

AppAcousticImagingStatus_t AppAcousticService_SetBinPolicy(AppAcousticImagingBinPolicy_t policy)
{
  if (AppAcousticService_BinPolicyIsValid(policy) == 0U)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  s_requested_bin_policy = policy;
  s_profile_change_pending = 1U;

  return APP_ACOUSTIC_IMAGING_OK;
}
