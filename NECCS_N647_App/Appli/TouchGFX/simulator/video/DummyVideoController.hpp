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

#ifndef TOUCHGFX_DUMMYVIDEOCONTROLLER_HPP
#define TOUCHGFX_DUMMYVIDEOCONTROLLER_HPP

#include <touchgfx/widgets/VideoWidget.hpp>

class DummyVideoController : public touchgfx::VideoController
{
public:
    DummyVideoController()
        : VideoController()
    {
    }

    Handle registerVideoWidget(touchgfx::VideoWidget& widget)
    {
        return 0;
    }

    void unregisterVideoWidget(const Handle handle)
    {
    }

    void setFrameRate(const Handle handle, uint32_t ui_frames, uint32_t video_frames)
    {
    }

    void setVideoData(const Handle handle, const uint8_t* movie, const uint32_t length)
    {
    }

    void setVideoData(const Handle handle, touchgfx::VideoDataReader& reader)
    {
    }

    void setCommand(const Handle handle, Command cmd, uint32_t param)
    {
    }

    bool updateFrame(const Handle handle, touchgfx::VideoWidget& widget)
    {
        return false;
    }

    void draw(const Handle handle, const touchgfx::Rect& invalidatedArea, const touchgfx::VideoWidget& widget)
    {
    }

    uint32_t getCurrentFrameNumber(const Handle handle)
    {
        return 0;
    }

    void getVideoInformation(const Handle handle, touchgfx::VideoInformation* data)
    {
    }

    bool getIsPlaying(const Handle handle)
    {
        return false;
    }

    void setVideoFrameRateCompensation(bool allow)
    {
    }
};

#endif // TOUCHGFX_DUMMYVIDEOCONTROLLER_HPP
