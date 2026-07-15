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

#include <gui/template_screen/TemplatePresenter.hpp>
#include <gui/template_screen/TemplateView.hpp>

TemplatePresenter::TemplatePresenter(TemplateView& v)
    : view(v)
{
}

void TemplatePresenter::activate()
{
    view.updateSnapshot(model->getSnapshot());
}

void TemplatePresenter::deactivate()
{
}

void TemplatePresenter::selectScreen(uint8_t screen)
{
    model->setActiveScreen(screen);
}

void TemplatePresenter::setMenuBlocksCamera(bool blocked)
{
    model->setMenuBlocksCamera(blocked);
}

void TemplatePresenter::rebootSystem()
{
    model->rebootSystem();
}

void TemplatePresenter::powerOffSystem()
{
    model->powerOffSystem();
}

void TemplatePresenter::selectProfile(uint8_t profile)
{
    model->setActiveProfile(profile);
}

void TemplatePresenter::cycleHeatPalette()
{
    model->cycleHeatPalette();
}

void TemplatePresenter::cycleScene()
{
    model->cycleScene();
}

void TemplatePresenter::toggleArrayMode()
{
    model->toggleArrayMode();
}

void TemplatePresenter::adjustTemperature(int8_t deltaC)
{
    model->adjustTemperature(deltaC);
}

void TemplatePresenter::requestScreenshot()
{
    model->requestScreenshot();
}

void TemplatePresenter::toggleRecording()
{
    model->toggleRecording();
}

void TemplatePresenter::refreshMedia()
{
    model->refreshMedia();
}

void TemplatePresenter::selectNextMedia()
{
    model->selectNextMedia();
}

void TemplatePresenter::readSelectedMedia()
{
    model->readSelectedMedia();
}

void TemplatePresenter::playToggleMedia()
{
    model->playToggleMedia();
}

void TemplatePresenter::requestThumbPage(uint32_t page)
{
    model->requestThumbPage(page);
}

void TemplatePresenter::selectMediaSlot(uint8_t slot)
{
    model->selectMediaSlot(slot);
}

void TemplatePresenter::toggleTrigger()
{
    model->toggleTrigger();
}

void TemplatePresenter::setBandHz(uint16_t loHz, uint16_t hiHz)
{
    model->setBandHz(loHz, hiHz);
}

void TemplatePresenter::adjustFieldParam(uint8_t param, int8_t dir)
{
    model->adjustFieldParam(param, dir);
}

void TemplatePresenter::toggleTrail()
{
    model->toggleTrail();
}

void TemplatePresenter::toggleBeamRecording()
{
    model->toggleBeamRecording();
}

void TemplatePresenter::beamAutoTrack()
{
    model->beamAutoTrack();
}

void TemplatePresenter::toggleLaser()
{
    model->toggleLaser();
}

void TemplatePresenter::setBeamManualTargetPx(int16_t px, int16_t py)
{
    model->setBeamManualTargetPx(px, py);
}

void TemplatePresenter::uiSnapshotUpdated(const AppUiSnapshot& snapshot)
{
    view.updateSnapshot(snapshot);
}
