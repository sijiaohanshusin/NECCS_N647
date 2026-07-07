#include "app_acoustic_service.h"

#include <stddef.h>
#include <string.h>

#include "app_pcmd_capture.h"
#include "app_bringup_thread.h"
#include "main.h"

#define APP_ACOUSTIC_SERVICE_TARGET_FPS          20U
#define APP_ACOUSTIC_SERVICE_MIN_FPS             10U
#define APP_ACOUSTIC_SERVICE_DEGRADE_LIMIT       3U
#define APP_ACOUSTIC_SERVICE_PROCESS_PERIOD_TICKS \
  (TX_TIMER_TICKS_PER_SECOND / APP_ACOUSTIC_SERVICE_TARGET_FPS)
#define APP_ACOUSTIC_SERVICE_PREVIEW_OVERLAY_ENABLE 1U
#define APP_ACOUSTIC_SERVICE_PREVIEW_MIN_CONFIDENCE 0.70f
#define APP_ACOUSTIC_SERVICE_SAMPLE_COUNT \
  (APP_MIC_ARRAY_PHYSICAL_MIC_COUNT * APP_AUDIO_FRAME_DEFAULT_WIDE32_FRAME_LEN)
#define APP_ACOUSTIC_SERVICE_CAMERA_HFOV_HALF_DEG \
  (APP_ACOUSTIC_SERVICE_CAMERA_HFOV_DEG * 0.5f)
#define APP_ACOUSTIC_SERVICE_CAMERA_VFOV_HALF_DEG \
  (APP_ACOUSTIC_SERVICE_CAMERA_VFOV_DEG * 0.5f)

static AppAcousticSrpContext_t s_srp_ctx __attribute__((aligned(32)));
static AppAcousticImagingVisFrame_t s_vis_frame __attribute__((aligned(32)));
static AppAcousticServiceSnapshot_t s_snapshot __attribute__((aligned(32)));
static float s_service_samples[APP_ACOUSTIC_SERVICE_SAMPLE_COUNT]
    __attribute__((section(".EXTRAM"), aligned(32)));

volatile uint32_t g_app_acoustic_service_disable_auto_degrade;

static volatile AppAcousticImagingRunMode_t s_requested_mode = APP_ACOUSTIC_IMAGING_MODE_STANDARD;
static volatile AppAcousticImagingProfile_t s_requested_profile = APP_ACOUSTIC_IMAGING_PROFILE_BALANCED;
static volatile AppAcousticImagingBinPolicy_t s_requested_bin_policy =
    APP_ACOUSTIC_IMAGING_BIN_POLICY_PROFILE_DEFAULT;
static volatile uint8_t s_profile_change_pending;
static AppAcousticImagingRunMode_t s_active_mode = APP_ACOUSTIC_IMAGING_MODE_STANDARD;
static AppAcousticImagingProfile_t s_active_profile = APP_ACOUSTIC_IMAGING_PROFILE_BALANCED;
static uint8_t s_have_input_seq;
static uint32_t s_last_input_seq;
static uint32_t s_over_budget_count;
static uint32_t s_error_count;
static uint32_t s_fps_window_ms;
static uint32_t s_fps_window_frames;
static uint16_t s_effective_fps_x10;

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

  status = App_AcousticSrp_Init(&s_srp_ctx, &config, APP_ACOUSTIC_BACKEND_F32_CMSIS);
  if (status == APP_ACOUSTIC_IMAGING_OK)
  {
    s_active_mode = mode;
    s_active_profile = config.profile;
    s_requested_profile = config.profile;
    s_over_budget_count = 0U;
    s_error_count = 0U;
    s_profile_change_pending = 0U;
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
    memset(snapshot->heat, 0, sizeof(snapshot->heat));
    snapshot->peak_index = 40U;
  }
}

static uint8_t AppAcousticService_MapCameraCell(float theta_deg,
                                                float phi_deg,
                                                uint32_t *cell_index)
{
  float col_f;
  float row_f;
  int32_t col;
  int32_t row;

  if ((cell_index == NULL) ||
      (theta_deg < -APP_ACOUSTIC_SERVICE_CAMERA_HFOV_HALF_DEG) ||
      (theta_deg > APP_ACOUSTIC_SERVICE_CAMERA_HFOV_HALF_DEG) ||
      (phi_deg < -APP_ACOUSTIC_SERVICE_CAMERA_VFOV_HALF_DEG) ||
      (phi_deg > APP_ACOUSTIC_SERVICE_CAMERA_VFOV_HALF_DEG))
  {
    return 0U;
  }

  col_f = ((theta_deg + APP_ACOUSTIC_SERVICE_CAMERA_HFOV_HALF_DEG) *
           (float)(APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE - 1U)) /
          APP_ACOUSTIC_SERVICE_CAMERA_HFOV_DEG;
  row_f = ((APP_ACOUSTIC_SERVICE_CAMERA_VFOV_HALF_DEG - phi_deg) *
           (float)(APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE - 1U)) /
          APP_ACOUSTIC_SERVICE_CAMERA_VFOV_DEG;

  col = (int32_t)(col_f + 0.5f);
  row = (int32_t)(row_f + 0.5f);
  if (col < 0)
  {
    col = 0;
  }
  else if (col >= (int32_t)APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE)
  {
    col = (int32_t)APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE - 1;
  }
  if (row < 0)
  {
    row = 0;
  }
  else if (row >= (int32_t)APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE)
  {
    row = (int32_t)APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE - 1;
  }

  *cell_index = (uint32_t)((row * (int32_t)APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE) + col);
  return 1U;
}

static void AppAcousticService_FillCameraHeat(AppAcousticServiceSnapshot_t *snapshot,
                                              const AppAcousticImagingVisFrame_t *vis_frame)
{
  float confidence;
  float peak_abs;
  uint32_t best_cell = 40U;
  uint8_t best_value = 0U;

  if ((snapshot == NULL) || (vis_frame == NULL))
  {
    return;
  }

  AppAcousticService_ClearHeat(snapshot);
  peak_abs = AppAcousticService_AbsF32(vis_frame->peak_value);
  if (peak_abs < 1.0e-9f)
  {
    return;
  }

  confidence = AppAcousticService_Clamp01(vis_frame->quality);
  if (vis_frame->valid != 0U)
  {
    confidence = 0.35f + (0.65f * confidence);
  }
  else
  {
    confidence *= 0.20f;
#if APP_ACOUSTIC_SERVICE_PREVIEW_OVERLAY_ENABLE
    if (confidence < APP_ACOUSTIC_SERVICE_PREVIEW_MIN_CONFIDENCE)
    {
      confidence = APP_ACOUSTIC_SERVICE_PREVIEW_MIN_CONFIDENCE;
    }
#endif
  }

  for (uint32_t i = 0U; i < APP_ACOUSTIC_IMAGING_COARSE_TOTAL; i++)
  {
    uint32_t cell;
    float norm;
    uint32_t scaled;

    if (AppAcousticService_MapCameraCell(vis_frame->theta_deg[i],
                                         vis_frame->phi_deg[i],
                                         &cell) == 0U)
    {
      continue;
    }

    norm = AppAcousticService_Clamp01(AppAcousticService_AbsF32(vis_frame->power[i]) / peak_abs);
    scaled = (uint32_t)((norm * confidence * 255.0f) + 0.5f);
    if (scaled > 255U)
    {
      scaled = 255U;
    }
    if ((uint8_t)scaled > snapshot->heat[cell])
    {
      snapshot->heat[cell] = (uint8_t)scaled;
    }
    if ((uint8_t)scaled > best_value)
    {
      best_value = (uint8_t)scaled;
      best_cell = cell;
    }
  }

  snapshot->peak_index = (uint8_t)best_cell;
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
  AppAcousticService_FillCameraHeat(snapshot, vis_frame);
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
  AppAcousticServiceSnapshot_t snapshot;
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

  memset(&snapshot, 0, sizeof(snapshot));
  snapshot.requested_mode = mode;
  snapshot.active_mode = mode;
  snapshot.requested_profile = s_requested_profile;
  snapshot.active_profile = s_requested_profile;
  snapshot.requested_bin_policy = s_requested_bin_policy;
  snapshot.backend = APP_ACOUSTIC_BACKEND_F32_CMSIS;
  snapshot.service_status = APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME;
  snapshot.peak_index = 40U;

  status = AppAcousticService_InitRuntime(mode, s_requested_bin_policy);
  snapshot.initialized = (status == APP_ACOUSTIC_IMAGING_OK) ? 1U : 0U;
  snapshot.last_status = (int32_t)status;
  snapshot.active_mode = s_active_mode;
  snapshot.requested_profile = s_requested_profile;
  snapshot.active_profile = s_active_profile;
  snapshot.active_bin_policy = s_srp_ctx.config.bin_policy;
  snapshot.active_channel_mask = s_srp_ctx.active_channel_mask;
  snapshot.pair_count = s_srp_ctx.pair_count;
  snapshot.grid_count = s_srp_ctx.grid_count;
  snapshot.active_bin_count = (uint16_t)s_srp_ctx.active_bin_count;
  AppAcousticService_PublishSnapshot(&snapshot);

  return status;
}

void AppAcousticService_ThreadEntry(ULONG thread_input)
{
  AppAudioFrame_t capture_frame;
  AppAudioFrame_t work_frame;
  AppAcousticServiceSnapshot_t snapshot;
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

    AppAcousticService_LoadSnapshot(&snapshot);
    snapshot.running = 1U;
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
    snapshot.requested_mode = s_requested_mode;
    snapshot.requested_profile = s_requested_profile;
    snapshot.requested_bin_policy = s_requested_bin_policy;

    if ((s_profile_change_pending != 0U) ||
        (s_srp_ctx.initialized == 0U) ||
        (s_active_mode != s_requested_mode) ||
        (s_srp_ctx.config.bin_policy !=
         App_AcousticImaging_ResolveBinPolicy(s_requested_profile, s_requested_bin_policy)))
    {
      status = AppAcousticService_InitRuntime(s_requested_mode, s_requested_bin_policy);
      snapshot.initialized = (status == APP_ACOUSTIC_IMAGING_OK) ? 1U : 0U;
      snapshot.last_status = (int32_t)status;
      snapshot.service_status = (status == APP_ACOUSTIC_IMAGING_OK) ?
                                APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME :
                                (int32_t)status;
      snapshot.active_mode = s_active_mode;
      snapshot.active_profile = s_active_profile;
      snapshot.active_bin_policy = s_srp_ctx.config.bin_policy;
      snapshot.active_channel_mask = s_srp_ctx.active_channel_mask;
      snapshot.pair_count = s_srp_ctx.pair_count;
      snapshot.grid_count = s_srp_ctx.grid_count;
      snapshot.active_bin_count = (uint16_t)s_srp_ctx.active_bin_count;
      AppAcousticService_PublishSnapshot(&snapshot);
      if (status != APP_ACOUSTIC_IMAGING_OK)
      {
        tx_thread_sleep(1U);
        continue;
      }
    }

    if (AppPcmdCapture_GetLatestAudioFrame(&capture_frame) == 0U)
    {
      snapshot.skipped_frames++;
      snapshot.valid = 0U;
      snapshot.service_status = APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME;
      AppAcousticService_ClearHeat(&snapshot);
      AppAcousticService_PublishSnapshot(&snapshot);
      App_BringUpStatus_Heartbeat(APP_BRINGUP_MODULE_ACOUSTIC,
                                  APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME);
      continue;
    }

    if ((s_have_input_seq != 0U) && (capture_frame.seq == s_last_input_seq))
    {
      snapshot.skipped_frames++;
      snapshot.input_seq = capture_frame.seq;
      snapshot.service_status = APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME;
      AppAcousticService_PublishSnapshot(&snapshot);
      App_BringUpStatus_Heartbeat(APP_BRINGUP_MODULE_ACOUSTIC,
                                  APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME);
      continue;
    }

    snapshot.input_seq = capture_frame.seq;
    if (AppAcousticService_CopyFrame(&work_frame, &capture_frame) == 0U)
    {
      snapshot.failed_frames++;
      snapshot.valid = 0U;
      snapshot.service_status = APP_ACOUSTIC_SERVICE_STATUS_COPY_FAILED;
      AppAcousticService_ClearHeat(&snapshot);
      AppAcousticService_PublishSnapshot(&snapshot);
      App_BringUpStatus_Fail(APP_BRINGUP_MODULE_ACOUSTIC,
                             APP_ACOUSTIC_SERVICE_STATUS_COPY_FAILED);
      continue;
    }

    {
      uint32_t process_start_ms = HAL_GetTick();
      status = App_AcousticSrp_ProcessFrame(&s_srp_ctx, &work_frame, &s_vis_frame);
      process_elapsed_ms = HAL_GetTick() - process_start_ms;
    }
    snapshot.last_status = (int32_t)status;
    if (status == APP_ACOUSTIC_IMAGING_OK)
    {
      snapshot.service_status = APP_ACOUSTIC_SERVICE_STATUS_OK;
      snapshot.processed_frames++;
      s_have_input_seq = 1U;
      s_last_input_seq = capture_frame.seq;
      AppAcousticService_FillVisSnapshot(&snapshot, &s_vis_frame, &s_srp_ctx);
      App_AcousticSrp_GetPerf(&s_srp_ctx, &snapshot.perf);
      AppAcousticService_UpdatePerf(&snapshot, &snapshot.perf, process_elapsed_ms);
      AppAcousticService_UpdateFps(&snapshot);
      App_BringUpStatus_Ready(APP_BRINGUP_MODULE_ACOUSTIC, (int32_t)status);
    }
    else
    {
      snapshot.failed_frames++;
      snapshot.valid = 0U;
      snapshot.service_status = (int32_t)status;
      AppAcousticService_ClearHeat(&snapshot);
      App_BringUpStatus_Fail(APP_BRINGUP_MODULE_ACOUSTIC, (int32_t)status);
    }

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
