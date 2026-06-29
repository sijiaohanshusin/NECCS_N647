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

#ifndef MODEL_HPP
#define MODEL_HPP

#include <stdint.h>

class ModelListener;

enum AppUiScreen
{
    APP_UI_SCREEN_IMAGE = 0,
    APP_UI_SCREEN_MICS = 1,
    APP_UI_SCREEN_PERF = 2,
    APP_UI_SCREEN_SETTINGS = 3
};

enum AppUiProfile
{
    APP_UI_PROFILE_FAST = 0,
    APP_UI_PROFILE_BALANCED = 1,
    APP_UI_PROFILE_QUALITY = 2
};

struct AppUiSnapshot
{
    uint32_t frameSeq;
    uint16_t touchX;
    uint16_t touchY;
    uint16_t touchRawX;
    uint16_t touchRawY;
    uint16_t srpMsX100;
    uint16_t uiFpsX10;
    int16_t thetaDeg;
    int16_t phiDeg;
    uint8_t activeScreen;
    uint8_t activeProfile;
    uint8_t touchReady;
    uint8_t touchDown;
    uint8_t touchIc;
    uint8_t qualityPct;
    uint8_t contrastPct;
    uint8_t peakIndex;
    uint8_t heat[81];
    uint8_t micLevel[32];
    uint8_t perfLoad[5];
};

/**
 * The Model class defines the data model in the model-view-presenter paradigm.
 * The Model is a singular object used across all presenters. The currently active
 * presenter will have a pointer to the Model through deriving from ModelListener.
 *
 * The Model will typically contain UI state information that must be kept alive
 * through screen transitions. It also usually provides the interface to the rest
 * of the system (the backend). As such, the Model can receive events and data from
 * the backend and inform the current presenter of such events through the modelListener
 * pointer, which is automatically configured to point to the current presenter.
 * Conversely, the current presenter can trigger events in the backend through the Model.
 */
class Model
{
public:
    Model();

    /**
     * Sets the modelListener to point to the currently active presenter. Called automatically
     * when switching screen.
     */
    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    /**
     * This function will be called automatically every frame. Can be used to e.g. sample hardware
     * peripherals or read events from the surrounding system and inject events to the GUI through
     * the ModelListener interface.
     */
    void tick();

    void setActiveScreen(uint8_t screen);
    void setActiveProfile(uint8_t profile);

    const AppUiSnapshot& getSnapshot() const
    {
        return snapshot;
    }

protected:
    /**
     * Pointer to the currently active presenter.
     */
    ModelListener* modelListener;
    AppUiSnapshot snapshot;
    uint32_t tickCount;
};

#endif /* MODEL_HPP */
