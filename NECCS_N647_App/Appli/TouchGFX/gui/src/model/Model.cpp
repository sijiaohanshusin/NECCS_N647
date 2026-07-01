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
    uint64_t total_bytes;
    uint64_t free_bytes;
    char last_file[32];
    char selected_file[32];
} AppMediaStatus_t;

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
#endif

namespace
{
uint8_t clampPercent(uint32_t value)
{
    return (value > 100U) ? 100U : static_cast<uint8_t>(value);
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

    const uint32_t peakX = (tickCount / 5U) % 9U;
    const int32_t peakYOffset = static_cast<int32_t>((tickCount / 17U) % 3U) - 1;
    const uint32_t peakY = static_cast<uint32_t>(4 + peakYOffset);
    snapshot.peakIndex = static_cast<uint8_t>((peakY * 9U) + peakX);

    for (uint32_t row = 0U; row < 9U; ++row)
    {
        for (uint32_t col = 0U; col < 9U; ++col)
        {
            const int32_t dx = static_cast<int32_t>(col) - static_cast<int32_t>(peakX);
            const int32_t dy = static_cast<int32_t>(row) - static_cast<int32_t>(peakY);
            const uint32_t dist = static_cast<uint32_t>((dx * dx) + (dy * dy));
            const uint32_t shoulder = ((row + col + (tickCount / 9U)) % 5U) * 7U;
            const int32_t value = 236 - static_cast<int32_t>(dist * 34U) + static_cast<int32_t>(shoulder);
            snapshot.heat[(row * 9U) + col] = (value <= 0) ? 18U : ((value > 255) ? 255U : static_cast<uint8_t>(value));
        }
    }

    for (uint32_t i = 0U; i < 32U; ++i)
    {
        const uint32_t wave = ((tickCount / 2U) + (i * 11U)) % 97U;
        snapshot.micLevel[i] = clampPercent(18U + ((wave * (i + 5U)) % 78U));
    }

    snapshot.perfLoad[0] = clampPercent(18U + ((tickCount / 3U) % 18U));
    snapshot.perfLoad[1] = clampPercent(34U + ((tickCount / 5U) % 28U));
    snapshot.perfLoad[2] = clampPercent(26U + ((tickCount / 7U) % 30U));
    snapshot.perfLoad[3] = clampPercent(12U + ((tickCount / 11U) % 22U));
    snapshot.perfLoad[4] = clampPercent(42U + ((tickCount / 13U) % 18U));

    snapshot.thetaDeg = static_cast<int16_t>(((tickCount * 3U) % 121U) - 60U);
    snapshot.phiDeg = static_cast<int16_t>(((tickCount * 2U) % 61U) - 30U);
    snapshot.qualityPct = static_cast<uint8_t>(68U + ((tickCount / 4U) % 24U));
    snapshot.contrastPct = static_cast<uint8_t>(42U + ((tickCount / 6U) % 34U));
    snapshot.srpMsX100 = static_cast<uint16_t>(560U + ((tickCount / 5U) % 220U));
    snapshot.uiFpsX10 = 200U;

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
        if (profile == APP_UI_PROFILE_FAST)
        {
            snapshot.srpMsX100 = 390U;
        }
        else if (profile == APP_UI_PROFILE_QUALITY)
        {
            snapshot.srpMsX100 = 930U;
        }
        else
        {
            snapshot.srpMsX100 = 640U;
        }

        if (modelListener != 0)
        {
            modelListener->uiSnapshotUpdated(snapshot);
        }
    }
}
