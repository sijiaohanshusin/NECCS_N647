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
extern "C"
{
#include "app_power.h"
#include "app_media.h"
#include "app_acoustic_service.h"
#include "app_camera_display.h"
#include "app_pcmd_capture.h"
#include "TOUCH/app_touch.h"
}
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
    uint8_t peak_index;
    uint8_t heat[81];
    uint8_t perf_load[5];
} AppAcousticServiceSnapshot_t;

static void AppAcousticService_GetSnapshot(AppAcousticServiceSnapshot_t* snapshot)
{
    if (snapshot != 0)
    {
        memset(snapshot, 0, sizeof(*snapshot));
    }
}

static int32_t AppAcousticService_SetProfile(AppAcousticImagingProfile_t profile)
{
    (void)profile;
    return 0;
}

static void AppCameraDisplay_SetAcousticOverlay(const uint8_t* heat,
                                                uint32_t count,
                                                uint8_t peakIndex,
                                                uint8_t qualityPct,
                                                uint8_t enabled)
{
    (void)heat;
    (void)count;
    (void)peakIndex;
    (void)qualityPct;
    (void)enabled;
}
#endif

namespace
{
constexpr uint8_t APP_UI_ACOUSTIC_OVERLAY_MIN_QUALITY = 10U;

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
}

Model::Model()
    : modelListener(0),
      tickCount(0U)
{
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.activeScreen = APP_UI_SCREEN_IMAGE;
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

    AppAcousticServiceSnapshot_t acoustic;
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
    snapshot.peakIndex = acoustic.peak_index;
    snapshot.srpMsX100 = acoustic.srp_ms_x100;
    snapshot.uiFpsX10 = (acoustic.fps_x10 != 0U) ? acoustic.fps_x10 : 200U;
    memcpy(snapshot.heat, acoustic.heat, sizeof(snapshot.heat));
    memcpy(snapshot.perfLoad, acoustic.perf_load, sizeof(snapshot.perfLoad));
    const bool acousticOverlayEnabled =
        (snapshot.activeScreen == APP_UI_SCREEN_IMAGE) &&
        (acoustic.valid != 0U) &&
        (acoustic.quality_pct >= APP_UI_ACOUSTIC_OVERLAY_MIN_QUALITY);
    AppCameraDisplay_SetAcousticOverlay(snapshot.heat,
                                        sizeof(snapshot.heat),
                                        snapshot.peakIndex,
                                        snapshot.qualityPct,
                                        acousticOverlayEnabled ? 1U : 0U);
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
    snapshot.mediaSelectedType = static_cast<uint8_t>(media.selected_type);
    copyFileName(snapshot.mediaLastFile, media.last_file, sizeof(snapshot.mediaLastFile));
    copyFileName(snapshot.mediaSelectedFile, media.selected_file, sizeof(snapshot.mediaSelectedFile));

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
