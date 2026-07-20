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

#ifndef TEMPLATE_PRESENTER_HPP
#define TEMPLATE_PRESENTER_HPP

#include <mvp/Presenter.hpp>
#include <gui/model/ModelListener.hpp>

class TemplateView;

/**
 * The Presenter for a template screen. In the MVP paradigm, the presenter acts upon
 * events from the model and the view. By deriving from ModelListener, the presenter
 * implements the interface which the model uses to inform the current presenter of
 * events and model state changes. The presenter also contains a reference to the
 * associated view, and uses this to update the view accordingly.
 *
 */
class TemplatePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    TemplatePresenter(TemplateView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    void selectScreen(uint8_t screen);
    void selectProfile(uint8_t profile);
    void cycleScene();
    void toggleArrayMode();
    void adjustTemperature(int8_t deltaC);
    void cycleHeatPalette();
    void setBandHz(uint16_t loHz, uint16_t hiHz);
    void toggleBandMode();
    void toggleUsbStorage();
    void adjustFieldParam(uint8_t param, int8_t dir);
    void toggleTrigger();
    void toggleTrail();
    void requestScreenshot();
    void toggleRecording();
    /* Directional recording (beamformer). */
    void toggleBeamRecording();
    void beamAutoTrack();
    void setBeamManualTargetPx(int16_t px, int16_t py);
    /* Laser pointer (gimbal skeleton). */
    void toggleLaser();
    void refreshMedia();
    void selectNextMedia();
    void readSelectedMedia();
    void playToggleMedia();
    void requestThumbPage(uint32_t page);
    void selectMediaSlot(uint8_t slot);
    void setMenuBlocksCamera(bool blocked);
    void rebootSystem();
    void powerOffSystem();
    virtual void uiSnapshotUpdated(const AppUiSnapshot& snapshot);

    virtual ~TemplatePresenter()
    {
    }

private:
    TemplatePresenter();

    TemplateView& view;
};

#endif // TEMPLATE_PRESENTER_HPP
