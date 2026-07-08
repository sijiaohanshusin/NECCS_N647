/******************************************************************************
* Copyright (c) 2018(-2026) STMicroelectronics.
* All rights reserved.
*
* This file is part of the TouchGFX 4.26.1 distribution.
*
* This software is licensed under terms that can be found in the LICENSE file in
* the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
*******************************************************************************/

#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#include <string.h>

#if defined(STM32N647xx)
/* The UI reaches the application layer exclusively through this facade. */
#include "app_ui_bridge.h"
#else
typedef struct
{
    uint8_t ready;
    uint8_t down;
    uint8_t ic;
    uint16_t x;
    uint16_t y;
    uint16_t raw_x;
    uint16_t raw_y;
} AppTouchSnapshot_t;

static void AppTouch_GetSnapshot(AppTouchSnapshot_t* snapshot)
{
    if (snapshot != 0)
    {
        memset(snapshot, 0, sizeof(*snapshot));
    }
}

typedef struct
{
    uint32_t flags;
    uint32_t battery_mv;
    uint32_t system_mv;
    int32_t battery_current_ma;
    uint16_t charger_status;
    uint32_t pin_state;
    uint8_t battery_percent;
    uint8_t state;
} AppPowerSnapshot_t;

static void AppPower_GetSnapshot(AppPowerSnapshot_t* snapshot)
{
    if (snapshot != 0)
    {
        memset(snapshot, 0, sizeof(*snapshot));
        snapshot->state = APP_UI_POWER_STATE_UNKNOWN;
    }
}

typedef struct
{
    uint32_t flags;
    uint32_t last_error;
    uint32_t sd_status;
    uint32_t mount_status;
    uint32_t format_status;
    uint32_t total_blocks;
    uint32_t media_blocks;
    uint32_t free_clusters;
    uint32_t screenshots;
    uint32_t videos;
    uint32_t selected_index;
    uint32_t selected_type;
    uint32_t record_frames;
    uint32_t dropped_frames;
    uint32_t record_seconds;
    uint32_t last_read_bytes;
    uint32_t preview_generation;
    uint32_t preview_type;
    uint32_t preview_width;
    uint32_t preview_height;
    uint32_t preview_frame_index;
    uint32_t preview_frame_count;
    uint64_t total_bytes;
    uint64_t free_bytes;
    char last_file[32];
    char selected_file[32];
} AppMediaStatus_t;

typedef struct
{
    uint32_t generation;
    uint32_t valid;
    uint32_t type;
    uint32_t width;
    uint32_t height;
    uint32_t frame_index;
    uint32_t frame_count;
} AppMediaPreviewInfo_t;

static uint32_t AppMedia_RequestScreenshot()
{
    return 0U;
}

static uint32_t AppMedia_RequestRecordStart()
{
    return 0U;
}

static uint32_t AppMedia_RequestRecordStop()
{
    return 0U;
}

static uint32_t AppMedia_RequestRefresh()
{
    return 0U;
}

static uint32_t AppMedia_RequestSelectNext()
{
    return 0U;
}

static uint32_t AppMedia_RequestReadSelected()
{
    return 0U;
}

static uint32_t AppMedia_RequestPlayToggle()
{
    return 0U;
}

static void AppMedia_GetStatus(AppMediaStatus_t* status)
{
    if (status != 0)
    {
        memset(status, 0, sizeof(*status));
    }
}

static const uint16_t* AppMedia_GetPreviewBuffer(AppMediaPreviewInfo_t* info)
{
    if (info != 0)
    {
        memset(info, 0, sizeof(*info));
    }
    return 0;
}

typedef struct
{
    uint8_t initialized;
    uint8_t started;
    uint8_t latest_frame_valid;
    uint8_t debug_ui_enabled;
    uint8_t raw_audio_valid;
    uint8_t raw_active_slot_count;
    uint8_t device_present_mask;
    uint8_t device_config_ok_mask;
    uint8_t device_status_ok_mask;
    uint8_t mic_level[32];
    int8_t mic_dbfs[32];
    uint32_t published_frames;
    uint32_t dropped_halves;
    uint32_t sync_miss_count;
    uint32_t raw_quality_flags;
    uint32_t raw_rail_sample_count;
    uint32_t raw_total_sample_count;
    uint16_t published_fps_x10;
    uint16_t raw_rail_percent_x10;
    int8_t raw_peak_dbfs;
    int8_t raw_avg_dbfs;
} AppPcmdCaptureSnapshot_t;

static void AppPcmdCapture_GetSnapshot(AppPcmdCaptureSnapshot_t* snapshot)
{
    if (snapshot != 0)
    {
        memset(snapshot, 0, sizeof(*snapshot));
    }
}

typedef enum
{
    APP_ACOUSTIC_IMAGING_PROFILE_FAST = 0,
    APP_ACOUSTIC_IMAGING_PROFILE_BALANCED = 1,
    APP_ACOUSTIC_IMAGING_PROFILE_QUALITY = 2
} AppAcousticImagingProfile_t;

#define APP_ACOUSTIC_SERVICE_FIELD_W 96U
#define APP_ACOUSTIC_SERVICE_FIELD_H 72U
#define APP_ACOUSTIC_SERVICE_FIELD_COUNT (APP_ACOUSTIC_SERVICE_FIELD_W * APP_ACOUSTIC_SERVICE_FIELD_H)
#define APP_ACOUSTIC_SERVICE_CAND_MAX 3U
#define APP_ACOUSTIC_SERVICE_SPECTRUM_BINS 64U
#define APP_ACOUSTIC_SERVICE_CAMERA_HFOV_DEG 77.0f
#define APP_ACOUSTIC_SERVICE_CAMERA_VFOV_DEG 61.1f

typedef struct
{
    uint8_t initialized;
    uint8_t running;
    uint8_t valid;
    uint8_t auto_degraded;
    AppAcousticImagingProfile_t active_profile;
    AppAcousticImagingProfile_t requested_profile;
    int32_t last_status;
    int32_t service_status;
    uint32_t input_seq;
    uint32_t output_seq;
    uint32_t processed_frames;
    uint32_t skipped_frames;
    uint32_t failed_frames;
    uint32_t degraded_count;
    uint32_t active_channel_mask;
    uint16_t srp_ms_x100;
    uint16_t fps_x10;
    int16_t theta_deg;
    int16_t phi_deg;
    uint8_t quality_pct;
    uint8_t contrast_pct;
    uint8_t scene;
    int8_t temperature_c;
    uint16_t band_lo_hz;
    uint16_t band_hi_hz;
    uint16_t speed_mps_x10;
    uint8_t cand_count;
    int16_t cand_theta[APP_ACOUSTIC_SERVICE_CAND_MAX];
    int16_t cand_phi[APP_ACOUSTIC_SERVICE_CAND_MAX];
    uint8_t cand_strength[APP_ACOUSTIC_SERVICE_CAND_MAX];
    uint8_t spectrum[APP_ACOUSTIC_SERVICE_SPECTRUM_BINS];
    uint8_t spectrum_peak_bin;
    uint8_t field[APP_ACOUSTIC_SERVICE_FIELD_COUNT];
    uint8_t perf_load[5];
} AppAcousticServiceSnapshot_t;

typedef enum
{
    APP_ACOUSTIC_SCENE_GENERAL = 0,
    APP_ACOUSTIC_SCENE_COUNT = 4
} AppAcousticScene_t;

static void AppAcousticService_GetSnapshot(AppAcousticServiceSnapshot_t* snapshot)
{
    if (snapshot != 0)
    {
        memset(snapshot, 0, sizeof(*snapshot));
        snapshot->temperature_c = 25;
        snapshot->band_lo_hz = 563U;
        snapshot->band_hi_hz = 7875U;
        snapshot->speed_mps_x10 = 3465U;
    }
}

static int32_t AppAcousticService_SetProfile(AppAcousticImagingProfile_t profile)
{
    (void)profile;
    return 0;
}

static int32_t AppAcousticService_SetScene(AppAcousticScene_t scene)
{
    (void)scene;
    return 0;
}

static int32_t AppAcousticService_SetTemperature(int8_t temperatureC)
{
    (void)temperatureC;
    return 0;
}

static int32_t AppAcousticService_SetBandHz(uint16_t loHz, uint16_t hiHz)
{
    (void)loHz;
    (void)hiHz;
    return 0;
}

typedef struct
{
    float db_floor;
    float gamma;
    float noise_gate;
    float bg_gain;
    uint32_t smooth_passes;
} AppAcousticFieldParams_t;

static AppAcousticFieldParams_t s_sim_field_params = {-15.0f, 1.10f, 0.10f, 1.45f, 2U};

static void AppAcousticService_GetFieldParams(AppAcousticFieldParams_t* params)
{
    if (params != 0)
    {
        *params = s_sim_field_params;
    }
}

static void AppAcousticService_SetFieldParams(const AppAcousticFieldParams_t* params)
{
    if (params != 0)
    {
        s_sim_field_params = *params;
    }
}

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint8_t strength;
    int8_t level_dbfs;
    uint8_t level_valid;
} AppCameraDisplayMarker_t;

static void AppCameraDisplay_SetAcousticField(const uint8_t* field,
                                              uint32_t count,
                                              const AppCameraDisplayMarker_t* markers,
                                              uint8_t markerCount,
                                              uint8_t qualityPct,
                                              uint8_t enabled)
{
    (void)field;
    (void)count;
    (void)markers;
    (void)markerCount;
    (void)qualityPct;
    (void)enabled;
}

static void AppCameraDisplay_SetHeatPalette(uint8_t palette)
{
    (void)palette;
}

static uint8_t AppCameraDisplay_GetHeatPalette(void)
{
    return 0U;
}

static uint8_t s_simTrailEnabled = 0U;

static void AppCameraDisplay_SetTrailEnabled(uint8_t enabled)
{
    s_simTrailEnabled = enabled;
}

static uint8_t AppCameraDisplay_GetTrailEnabled(void)
{
    return s_simTrailEnabled;
}

static void AppCameraDisplay_SetVisible(uint8_t visible)
{
    (void)visible;
}

typedef enum
{
    APP_BRINGUP_MODULE_CLOCK = 0,
    APP_BRINGUP_MODULE_MEMORY,
    APP_BRINGUP_MODULE_DISPLAY,
    APP_BRINGUP_MODULE_TOUCH,
    APP_BRINGUP_MODULE_I2C,
    APP_BRINGUP_MODULE_CAMERA,
    APP_BRINGUP_MODULE_PCMD_RAW,
    APP_BRINGUP_MODULE_AUDIO_FRAME,
    APP_BRINGUP_MODULE_ACOUSTIC,
    APP_BRINGUP_MODULE_UI_OVERLAY,
    APP_BRINGUP_MODULE_MEDIA,
    APP_BRINGUP_MODULE_COUNT
} AppBringUpModule_t;

typedef struct
{
    uint32_t magic;
    uint32_t seq;
    uint32_t enabled_mask;
    uint32_t started_mask;
    uint32_t ready_mask;
    uint32_t failed_mask;
    uint32_t skipped_mask;
    uint32_t active_mask;
    uint32_t control_mask;
    uint32_t loop_count;
} AppBringUpSnapshot_t;

static void App_BringUpStatus_GetSnapshot(AppBringUpSnapshot_t* snapshot)
{
    if (snapshot != 0)
    {
        memset(snapshot, 0, sizeof(*snapshot));
        /* Simulator boots instantly with every module ready. */
        snapshot->enabled_mask = 0x7FFU;
        snapshot->ready_mask = 0x7FFU;
    }
}
#endif

namespace
{
/* Bring-up preview keeps the overlay visible even without a valid SRP
 * result; DEBUG-only so Release shows the overlay only for real detections. */
#ifdef DEBUG
constexpr bool APP_UI_ACOUSTIC_OVERLAY_PREVIEW_ENABLE = true;
#else
constexpr bool APP_UI_ACOUSTIC_OVERLAY_PREVIEW_ENABLE = false;
#endif
constexpr uint8_t APP_UI_ACOUSTIC_OVERLAY_MIN_QUALITY = 1U;

uint8_t clampPercent(uint32_t value)
{
    return (value > 100U) ? 100U : static_cast<uint8_t>(value);
}

AppAcousticImagingProfile_t acousticProfileFromUi(uint8_t profile)
{
    switch (profile)
    {
    case APP_UI_PROFILE_FAST:
        return APP_ACOUSTIC_IMAGING_PROFILE_FAST;
    case APP_UI_PROFILE_QUALITY:
        return APP_ACOUSTIC_IMAGING_PROFILE_QUALITY;
    default:
        return APP_ACOUSTIC_IMAGING_PROFILE_BALANCED;
    }
}

uint8_t uiProfileFromAcoustic(AppAcousticImagingProfile_t profile)
{
    switch (profile)
    {
    case APP_ACOUSTIC_IMAGING_PROFILE_FAST:
        return APP_UI_PROFILE_FAST;
    case APP_ACOUSTIC_IMAGING_PROFILE_QUALITY:
        return APP_UI_PROFILE_QUALITY;
    default:
        return APP_UI_PROFILE_BALANCED;
    }
}

void copyFileName(char* destination, const char* source, uint32_t length)
{
    if ((destination == 0) || (length == 0U))
    {
        return;
    }

    if (source == 0)
    {
        destination[0] = '\0';
        return;
    }

    strncpy(destination, source, length - 1U);
    destination[length - 1U] = '\0';
}

/* Poll the acoustic service, mirror it into the UI snapshot and drive the
 * camera-display overlay policy. */
void pollAcoustic(AppUiSnapshot& snapshot)
{
    /* ~1.9 KB with the heat field; static keeps it off the 4 KB TouchGFX
     * thread stack (Model::tick only ever runs on that one thread). */
    static AppAcousticServiceSnapshot_t acoustic;
    memset(&acoustic, 0, sizeof(acoustic));
    AppAcousticService_GetSnapshot(&acoustic);
    snapshot.acousticFlags = 0U;
    if (acoustic.initialized != 0U)
    {
        snapshot.acousticFlags |= APP_UI_ACOUSTIC_FLAG_INITIALIZED;
    }
    if (acoustic.running != 0U)
    {
        snapshot.acousticFlags |= APP_UI_ACOUSTIC_FLAG_RUNNING;
    }
    if (acoustic.valid != 0U)
    {
        snapshot.acousticFlags |= APP_UI_ACOUSTIC_FLAG_VALID;
    }
    if (acoustic.auto_degraded != 0U)
    {
        snapshot.acousticFlags |= APP_UI_ACOUSTIC_FLAG_AUTO_DEGRADED;
    }
    if ((acoustic.initialized != 0U) || (acoustic.running != 0U))
    {
        snapshot.activeProfile = uiProfileFromAcoustic(acoustic.active_profile);
    }
    snapshot.acousticInputSeq = acoustic.input_seq;
    snapshot.acousticOutputSeq = acoustic.output_seq;
    snapshot.acousticProcessedFrames = acoustic.processed_frames;
    snapshot.acousticSkippedFrames = acoustic.skipped_frames;
    snapshot.acousticFailedFrames = acoustic.failed_frames;
    snapshot.acousticDegradedCount = acoustic.degraded_count;
    snapshot.acousticActiveChannelMask = acoustic.active_channel_mask;
    snapshot.acousticLastStatus = acoustic.last_status;
    snapshot.thetaDeg = acoustic.theta_deg;
    snapshot.phiDeg = acoustic.phi_deg;
    snapshot.qualityPct = acoustic.quality_pct;
    snapshot.contrastPct = acoustic.contrast_pct;
    snapshot.srpMsX100 = acoustic.srp_ms_x100;
    snapshot.uiFpsX10 = acoustic.fps_x10;
    snapshot.srpPreprocessCycles = acoustic.perf.preprocess_cycles;
    snapshot.srpFftCycles = acoustic.perf.fft_cycles;
    snapshot.srpGccCycles = acoustic.perf.gcc_cycles;
    snapshot.srpCoarseCycles = acoustic.perf.coarse_cycles;
    snapshot.srpFineCycles = acoustic.perf.fine_cycles;
    snapshot.srpTotalCycles = acoustic.perf.total_cycles;
    memcpy(snapshot.perfLoad, acoustic.perf_load, sizeof(snapshot.perfLoad));

    snapshot.candCount = (acoustic.cand_count <= 3U) ? acoustic.cand_count : 3U;
    for (uint32_t i = 0U; i < 3U; ++i)
    {
        snapshot.candTheta[i] = (i < snapshot.candCount) ? acoustic.cand_theta[i] : 0;
        snapshot.candPhi[i] = (i < snapshot.candCount) ? acoustic.cand_phi[i] : 0;
        snapshot.candStrength[i] = (i < snapshot.candCount) ? acoustic.cand_strength[i] : 0U;
    }
    snapshot.heatPalette = AppCameraDisplay_GetHeatPalette();
    snapshot.acousticScene = acoustic.scene;
    snapshot.acousticTempC = acoustic.temperature_c;
    snapshot.acousticBandLoHz = acoustic.band_lo_hz;
    snapshot.acousticBandHiHz = acoustic.band_hi_hz;
    snapshot.acousticSpeedX10 = acoustic.speed_mps_x10;
    memcpy(snapshot.spectrum, acoustic.spectrum, sizeof(snapshot.spectrum));
    snapshot.spectrumPeakBin = acoustic.spectrum_peak_bin;

    {
        AppAcousticFieldParams_t fieldParams;
        AppAcousticService_GetFieldParams(&fieldParams);
        snapshot.fieldDbFloor = (int8_t)(fieldParams.db_floor - 0.5f);
        snapshot.fieldGammaX100 = (uint16_t)((fieldParams.gamma * 100.0f) + 0.5f);
        snapshot.fieldNoiseGateX100 = (uint8_t)((fieldParams.noise_gate * 100.0f) + 0.5f);
        snapshot.fieldSmoothPasses = (uint8_t)fieldParams.smooth_passes;
    }

    /* DEBUG preview keeps the overlay path exercised only until the first
     * real SRP frame lands (the field is all-zero until then, so the draw
     * cost is negligible); afterwards the overlay strictly follows valid
     * results. An always-on preview would re-blend a stale field at camera
     * rate and starve the SRP thread outright. */
    const bool acousticOverlayPreview =
        APP_UI_ACOUSTIC_OVERLAY_PREVIEW_ENABLE &&
        (acoustic.processed_frames == 0U);
    const bool acousticOverlayEnabled =
        (snapshot.activeScreen == APP_UI_SCREEN_IMAGE) &&
        (((acoustic.valid != 0U) &&
          (acoustic.quality_pct >= APP_UI_ACOUSTIC_OVERLAY_MIN_QUALITY)) ||
         acousticOverlayPreview);

    /* Candidate markers mapped from angles into camera-frame pixels. */
    AppCameraDisplayMarker_t markers[3];
    uint8_t markerCount = 0U;
    for (uint32_t i = 0U; i < snapshot.candCount; ++i)
    {
        const float theta = static_cast<float>(acoustic.cand_theta[i]);
        const float phi = static_cast<float>(acoustic.cand_phi[i]);
        const float halfH = APP_ACOUSTIC_SERVICE_CAMERA_HFOV_DEG * 0.5f;
        const float halfV = APP_ACOUSTIC_SERVICE_CAMERA_VFOV_DEG * 0.5f;

        if ((theta < -halfH) || (theta > halfH) || (phi < -halfV) || (phi > halfV))
        {
            continue;
        }

        const float px = ((theta + halfH) * 640.0f) / APP_ACOUSTIC_SERVICE_CAMERA_HFOV_DEG;
        const float py = ((halfV - phi) * 480.0f) / APP_ACOUSTIC_SERVICE_CAMERA_VFOV_DEG;
        markers[markerCount].x = static_cast<uint16_t>((px < 0.0f) ? 0.0f : ((px > 639.0f) ? 639.0f : px));
        markers[markerCount].y = static_cast<uint16_t>((py < 0.0f) ? 0.0f : ((py > 479.0f) ? 479.0f : py));
        markers[markerCount].strength = acoustic.cand_strength[i];
        /* The primary cross carries the array peak level (dBFS estimate);
         * pcmdRawPeakDbfs is one tick stale, which is fine at 60 Hz. */
        markers[markerCount].level_dbfs = snapshot.pcmdRawPeakDbfs;
        markers[markerCount].level_valid =
            ((markerCount == 0U) &&
             (acoustic.valid != 0U) &&
             ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_RAW_VALID) != 0U)) ? 1U : 0U;
        ++markerCount;
    }

    AppCameraDisplay_SetAcousticField(acoustic.field,
                                      sizeof(acoustic.field),
                                      markers,
                                      markerCount,
                                      snapshot.qualityPct,
                                      acousticOverlayEnabled ? 1U : 0U);
}

void pollPcmd(AppUiSnapshot& snapshot)
{
    memset(snapshot.micLevel, 0, sizeof(snapshot.micLevel));
    memset(snapshot.micDbfs, -90, sizeof(snapshot.micDbfs));

    AppPcmdCaptureSnapshot_t pcmd;
    memset(&pcmd, 0, sizeof(pcmd));
    AppPcmdCapture_GetSnapshot(&pcmd);
    snapshot.pcmdFlags = 0U;
    if (pcmd.initialized != 0U)
    {
        snapshot.pcmdFlags |= APP_UI_PCMD_FLAG_INITIALIZED;
    }
    if (pcmd.started != 0U)
    {
        snapshot.pcmdFlags |= APP_UI_PCMD_FLAG_STARTED;
    }
    if (pcmd.latest_frame_valid != 0U)
    {
        snapshot.pcmdFlags |= APP_UI_PCMD_FLAG_FRAME_VALID;
    }
    if ((pcmd.latest_frame_valid != 0U) ||
        (pcmd.started != 0U) ||
        (pcmd.raw_active_slot_count != 0U) ||
        (pcmd.raw_quality_flags != 0U))
    {
        for (uint32_t i = 0U; i < 32U; ++i)
        {
            snapshot.micLevel[i] = clampPercent(pcmd.mic_level[i]);
            snapshot.micDbfs[i] = pcmd.mic_dbfs[i];
        }
    }
    if (pcmd.debug_ui_enabled != 0U)
    {
        snapshot.pcmdFlags |= APP_UI_PCMD_FLAG_DEBUG_ENABLED;
    }
    if (pcmd.raw_audio_valid != 0U)
    {
        snapshot.pcmdFlags |= APP_UI_PCMD_FLAG_RAW_VALID;
    }
    if ((pcmd.raw_quality_flags & APP_UI_PCMD_RAW_FLAG_RAIL_FAULT) != 0U)
    {
        snapshot.pcmdFlags |= APP_UI_PCMD_FLAG_RAW_FAULT;
    }
    if (pcmd.recovering != 0U)
    {
        snapshot.pcmdFlags |= APP_UI_PCMD_FLAG_RECOVERING;
    }
    snapshot.pcmdWatchdogRestarts = pcmd.watchdog_restart_count;
    snapshot.pcmdDebugEnabled = pcmd.debug_ui_enabled;
    snapshot.pcmdDevicePresentMask = pcmd.device_present_mask;
    snapshot.pcmdDeviceConfigOkMask = pcmd.device_config_ok_mask;
    snapshot.pcmdDeviceStatusOkMask = pcmd.device_status_ok_mask;
    snapshot.pcmdPublishedFrames = pcmd.published_frames;
    snapshot.pcmdDroppedHalves = pcmd.dropped_halves;
    snapshot.pcmdSyncMissCount = pcmd.sync_miss_count;
    snapshot.pcmdRawQualityFlags = pcmd.raw_quality_flags;
    snapshot.pcmdRawRailSampleCount = pcmd.raw_rail_sample_count;
    snapshot.pcmdRawTotalSampleCount = pcmd.raw_total_sample_count;
    snapshot.pcmdFpsX10 = pcmd.published_fps_x10;
    snapshot.pcmdRawRailPercentX10 = pcmd.raw_rail_percent_x10;
    snapshot.pcmdRawActiveSlotCount = pcmd.raw_active_slot_count;
    snapshot.pcmdRawPeakDbfs = pcmd.raw_peak_dbfs;
    snapshot.pcmdRawAvgDbfs = pcmd.raw_avg_dbfs;
}

void pollCameraDisplay(AppUiSnapshot& snapshot)
{
    AppCameraDisplayStatus_t display;
    memset(&display, 0, sizeof(display));
    AppCameraDisplay_GetStatus(&display);
    snapshot.cameraSwapCount = display.swap_count;
    snapshot.cameraOverlayDrawCount = display.overlay_draw_count;
    snapshot.cameraDma2dCopyCount = display.dma2d_copy_count;
    snapshot.cameraDisplayErrorCount = display.error_count;
    snapshot.cameraDma2dErrorCode = display.dma2d_error_code;
#if defined(STM32N647xx)
    snapshot.overlayDrawCycles = g_app_camera_overlay_draw_cycles;
#else
    snapshot.overlayDrawCycles = 0U;
#endif
}

void pollTouch(AppUiSnapshot& snapshot)
{
    AppTouchSnapshot_t touch;
    memset(&touch, 0, sizeof(touch));
    AppTouch_GetSnapshot(&touch);
    snapshot.touchReady = touch.ready;
    snapshot.touchDown = touch.down;
    snapshot.touchIc = static_cast<uint8_t>(touch.ic);
    snapshot.touchX = touch.x;
    snapshot.touchY = touch.y;
    snapshot.touchRawX = touch.raw_x;
    snapshot.touchRawY = touch.raw_y;
}

void pollPower(AppUiSnapshot& snapshot)
{
    AppPowerSnapshot_t power;
    memset(&power, 0, sizeof(power));
    AppPower_GetSnapshot(&power);
    snapshot.powerFlags = power.flags;
    snapshot.batteryMv = power.battery_mv;
    snapshot.systemMv = power.system_mv;
    snapshot.batteryCurrentMa = power.battery_current_ma;
    snapshot.chargerStatus = power.charger_status;
    snapshot.powerPinState = power.pin_state;
    snapshot.batteryPct = power.battery_percent;
    snapshot.powerState = power.state;
}

void pollBringup(AppUiSnapshot& snapshot)
{
    AppBringUpSnapshot_t bringup;
    memset(&bringup, 0, sizeof(bringup));
    App_BringUpStatus_GetSnapshot(&bringup);
    snapshot.bringupEnabledMask = bringup.enabled_mask;
    snapshot.bringupReadyMask = bringup.ready_mask;
    snapshot.bringupFailedMask = bringup.failed_mask;
    snapshot.bringupSkippedMask = bringup.skipped_mask;
}

void pollMedia(AppUiSnapshot& snapshot)
{
    AppMediaStatus_t media;
    memset(&media, 0, sizeof(media));
    AppMedia_GetStatus(&media);
    snapshot.mediaFlags = media.flags;
    snapshot.mediaLastError = media.last_error;
    snapshot.mediaScreenshots = media.screenshots;
    snapshot.mediaVideos = media.videos;
    snapshot.mediaRecordFrames = media.record_frames;
    snapshot.mediaDroppedFrames = media.dropped_frames;
    snapshot.mediaRecordSeconds = media.record_seconds;
    snapshot.mediaLastReadBytes = media.last_read_bytes;
    snapshot.mediaFreeMb = static_cast<uint32_t>(media.free_bytes / (1024ULL * 1024ULL));
    snapshot.mediaTotalMb = static_cast<uint32_t>(media.total_bytes / (1024ULL * 1024ULL));
    /* FileX occasionally reports a bogus huge available-cluster count right
     * after mount; clamp so the UI never shows free > total. */
    if (snapshot.mediaFreeMb > snapshot.mediaTotalMb)
    {
        snapshot.mediaFreeMb = snapshot.mediaTotalMb;
    }
    snapshot.mediaSelectedType = static_cast<uint8_t>(media.selected_type);
    copyFileName(snapshot.mediaLastFile, media.last_file, sizeof(snapshot.mediaLastFile));
    copyFileName(snapshot.mediaSelectedFile, media.selected_file, sizeof(snapshot.mediaSelectedFile));

#if defined(STM32N647xx) && defined(DEBUG)
    snapshot.mediaEncodeMs = static_cast<uint16_t>(g_app_media_perf_encode_ms);
#else
    snapshot.mediaEncodeMs = 0U;
#endif

    AppMediaPreviewInfo_t preview;
    memset(&preview, 0, sizeof(preview));
    snapshot.mediaPreviewPixels = AppMedia_GetPreviewBuffer(&preview);
    snapshot.mediaPreviewGeneration = preview.generation;
    snapshot.mediaPreviewValid = (preview.valid != 0U) ? 1U : 0U;
    snapshot.mediaPreviewType = static_cast<uint8_t>(preview.type);
    snapshot.mediaPreviewWidth = static_cast<uint16_t>(preview.width);
    snapshot.mediaPreviewHeight = static_cast<uint16_t>(preview.height);
    snapshot.mediaPreviewFrameIndex = preview.frame_index;
    snapshot.mediaPreviewFrameCount = preview.frame_count;
}
}

namespace
{
/* Boot page pacing: minimum splash time, and a hard cap after which the main
 * UI is entered regardless of module state (modules keep starting in the
 * background and their live status is visible on the main pages). The
 * acoustic engine is intentionally NOT watched: it only becomes ready after
 * the first valid PCMD frame (~10 s), which is too long to gate the UI on. */
constexpr uint32_t APP_UI_BOOT_MIN_TICKS = 168U;  /* ~2.8 s @60 Hz */
constexpr uint32_t APP_UI_BOOT_MAX_TICKS = 300U;  /* ~5 s @60 Hz */
constexpr uint32_t APP_UI_BOOT_WATCHED_MASK =
    (1UL << 4) |   /* I2C / power rail        */
    (1UL << 5) |   /* camera                  */
    (1UL << 6) |   /* PCMD mic array          */
    (1UL << 10);   /* media / filesystem      */
}

/* GDB-visible probe: {tickCount, bootTicks, activeScreen, bringup summary}. */
volatile uint32_t g_app_ui_debug[4];

/* GDB remote control: write 0..4 to switch pages (also used for scripted
 * demo runs); the Model consumes the request and resets it to 0xFF. */
volatile uint32_t g_app_ui_request_screen = 0xFFU;

Model::Model()
    : modelListener(0),
      tickCount(0U),
      bootTicks(0U),
      triggerCooldown(0U),
      triggerArmed(false),
      triggerPrevHot(false)
{
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.activeScreen = APP_UI_SCREEN_BOOT;
    snapshot.activeProfile = APP_UI_PROFILE_BALANCED;
    snapshot.uiFpsX10 = 200U;
    snapshot.srpMsX100 = 640U;
    copyFileName(snapshot.mediaLastFile, "", sizeof(snapshot.mediaLastFile));
    copyFileName(snapshot.mediaSelectedFile, "", sizeof(snapshot.mediaSelectedFile));
}

void Model::tick()
{
    ++tickCount;
    snapshot.frameSeq = tickCount;

    pollBringup(snapshot);
    pollAcoustic(snapshot);
    pollPcmd(snapshot);
    pollCameraDisplay(snapshot);
    pollTouch(snapshot);
    pollPower(snapshot);
    pollMedia(snapshot);

    if (snapshot.activeScreen == APP_UI_SCREEN_BOOT)
    {
        ++bootTicks;
        snapshot.bootElapsedMs = (bootTicks * 1000U) / 60U;

        const uint32_t watched = snapshot.bringupEnabledMask & APP_UI_BOOT_WATCHED_MASK;
        const uint32_t resolved = (snapshot.bringupReadyMask |
                                   snapshot.bringupFailedMask |
                                   snapshot.bringupSkippedMask) & watched;
        const bool allResolved = (watched != 0U) && (resolved == watched);
        if (((bootTicks >= APP_UI_BOOT_MIN_TICKS) && allResolved) ||
            (bootTicks >= APP_UI_BOOT_MAX_TICKS))
        {
            setActiveScreen(APP_UI_SCREEN_IMAGE);
        }
    }

    /* Acoustic trigger: on a rising "hot" edge (valid detection above the
     * level threshold) capture a screenshot, then hold off for ~6 s. */
    snapshot.trailEnabled = AppCameraDisplay_GetTrailEnabled();
    if (triggerCooldown > 0U)
    {
        --triggerCooldown;
    }
    if (triggerArmed && (snapshot.activeScreen != APP_UI_SCREEN_BOOT))
    {
        const bool hot =
            ((snapshot.acousticFlags & APP_UI_ACOUSTIC_FLAG_VALID) != 0U) &&
            (snapshot.qualityPct >= 12U) &&
            (snapshot.pcmdRawPeakDbfs >= -32);
        if (hot && !triggerPrevHot && (triggerCooldown == 0U))
        {
            requestScreenshot();
            ++snapshot.triggerCount;
            triggerCooldown = 360U;
        }
        triggerPrevHot = hot;
    }
    else
    {
        triggerPrevHot = false;
    }
    snapshot.triggerArmed = triggerArmed ? 1U : 0U;

    g_app_ui_debug[0] = tickCount;
    g_app_ui_debug[1] = bootTicks;
    g_app_ui_debug[2] = snapshot.activeScreen;
    g_app_ui_debug[3] = (snapshot.bringupEnabledMask << 16) | (snapshot.bringupReadyMask & 0xFFFFU);

    if ((g_app_ui_request_screen <= APP_UI_SCREEN_MEDIA) &&
        (snapshot.activeScreen != APP_UI_SCREEN_BOOT))
    {
        const uint8_t requested = static_cast<uint8_t>(g_app_ui_request_screen);
        g_app_ui_request_screen = 0xFFU;
        setActiveScreen(requested);
    }

    if ((modelListener != 0) && ((tickCount % 3U) == 0U))
    {
        modelListener->uiSnapshotUpdated(snapshot);
    }
}

void Model::setActiveScreen(uint8_t screen)
{
    if (screen <= APP_UI_SCREEN_MEDIA)
    {
        snapshot.activeScreen = screen;
        /* The camera LTDC layer is only shown on the image page; owning this
         * here keeps the View free of direct app-layer calls. */
        AppCameraDisplay_SetVisible((screen == APP_UI_SCREEN_IMAGE) ? 1U : 0U);
        if (modelListener != 0)
        {
            modelListener->uiSnapshotUpdated(snapshot);
        }
    }
}

void Model::requestScreenshot()
{
    (void)AppMedia_RequestScreenshot();
}

void Model::toggleRecording()
{
    if ((snapshot.mediaFlags & APP_UI_MEDIA_FLAG_RECORDING) != 0U)
    {
        (void)AppMedia_RequestRecordStop();
    }
    else
    {
        (void)AppMedia_RequestRecordStart();
    }
}

void Model::refreshMedia()
{
    (void)AppMedia_RequestRefresh();
}

void Model::selectNextMedia()
{
    (void)AppMedia_RequestSelectNext();
}

void Model::readSelectedMedia()
{
    (void)AppMedia_RequestReadSelected();
}

void Model::playToggleMedia()
{
    (void)AppMedia_RequestPlayToggle();
}

void Model::setActiveProfile(uint8_t profile)
{
    if (profile <= APP_UI_PROFILE_QUALITY)
    {
        snapshot.activeProfile = profile;
        (void)AppAcousticService_SetProfile(acousticProfileFromUi(profile));

        if (modelListener != 0)
        {
            modelListener->uiSnapshotUpdated(snapshot);
        }
    }
}

void Model::cycleHeatPalette()
{
    const uint8_t next = static_cast<uint8_t>((AppCameraDisplay_GetHeatPalette() + 1U) % 3U);
    AppCameraDisplay_SetHeatPalette(next);
    snapshot.heatPalette = next;
    if (modelListener != 0)
    {
        modelListener->uiSnapshotUpdated(snapshot);
    }
}

void Model::setBandHz(uint16_t loHz, uint16_t hiHz)
{
    (void)AppAcousticService_SetBandHz(loHz, hiHz);
    if (modelListener != 0)
    {
        modelListener->uiSnapshotUpdated(snapshot);
    }
}

void Model::adjustFieldParam(uint8_t param, int8_t dir)
{
    AppAcousticFieldParams_t params;
    const float step = (dir < 0) ? -1.0f : 1.0f;

    AppAcousticService_GetFieldParams(&params);
    switch (param)
    {
    case 0U:
        params.db_floor += step * 1.0f;
        break;
    case 1U:
        params.gamma += step * 0.05f;
        break;
    case 2U:
        params.noise_gate += step * 0.02f;
        break;
    case 3U:
    default:
        if ((dir < 0) && (params.smooth_passes == 0U))
        {
            return;
        }
        params.smooth_passes = (dir < 0) ? (params.smooth_passes - 1U)
                                         : (params.smooth_passes + 1U);
        break;
    }
    /* The service clamps every field to its valid range. */
    AppAcousticService_SetFieldParams(&params);
    pollAcoustic(snapshot);
    if (modelListener != 0)
    {
        modelListener->uiSnapshotUpdated(snapshot);
    }
}

void Model::toggleTrigger()
{
    triggerArmed = !triggerArmed;
    triggerPrevHot = false;
    triggerCooldown = 0U;
    snapshot.triggerArmed = triggerArmed ? 1U : 0U;
    if (modelListener != 0)
    {
        modelListener->uiSnapshotUpdated(snapshot);
    }
}

void Model::toggleTrail()
{
    const uint8_t next = (AppCameraDisplay_GetTrailEnabled() != 0U) ? 0U : 1U;
    AppCameraDisplay_SetTrailEnabled(next);
    snapshot.trailEnabled = next;
    if (modelListener != 0)
    {
        modelListener->uiSnapshotUpdated(snapshot);
    }
}

void Model::cycleScene()
{
    const uint8_t next = static_cast<uint8_t>((snapshot.acousticScene + 1U) %
                                              static_cast<uint8_t>(APP_ACOUSTIC_SCENE_COUNT));
    (void)AppAcousticService_SetScene(static_cast<AppAcousticScene_t>(next));
    /* Optimistic update; the service confirms via the next snapshot poll. */
    snapshot.acousticScene = next;
    if (modelListener != 0)
    {
        modelListener->uiSnapshotUpdated(snapshot);
    }
}

void Model::adjustTemperature(int8_t deltaC)
{
    int32_t next = static_cast<int32_t>(snapshot.acousticTempC) + deltaC;
    if (next < -20)
    {
        next = -20;
    }
    if (next > 60)
    {
        next = 60;
    }
    (void)AppAcousticService_SetTemperature(static_cast<int8_t>(next));
    snapshot.acousticTempC = static_cast<int8_t>(next);
    if (modelListener != 0)
    {
        modelListener->uiSnapshotUpdated(snapshot);
    }
}
