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

extern "C"
{
#include "TOUCH/app_touch.h"
}

namespace
{
uint8_t clampPercent(uint32_t value)
{
    return (value > 100U) ? 100U : static_cast<uint8_t>(value);
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

    AppTouchSnapshot_t touch = {0};
    AppTouch_GetSnapshot(&touch);
    snapshot.touchReady = touch.ready;
    snapshot.touchDown = touch.down;
    snapshot.touchIc = static_cast<uint8_t>(touch.ic);
    snapshot.touchX = touch.x;
    snapshot.touchY = touch.y;
    snapshot.touchRawX = touch.raw_x;
    snapshot.touchRawY = touch.raw_y;

    if ((modelListener != 0) && ((tickCount % 3U) == 0U))
    {
        modelListener->uiSnapshotUpdated(snapshot);
    }
}

void Model::setActiveScreen(uint8_t screen)
{
    if (screen <= APP_UI_SCREEN_SETTINGS)
    {
        snapshot.activeScreen = screen;
        if (modelListener != 0)
        {
            modelListener->uiSnapshotUpdated(snapshot);
        }
    }
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
