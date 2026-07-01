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

void TemplatePresenter::selectProfile(uint8_t profile)
{
    model->setActiveProfile(profile);
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

void TemplatePresenter::uiSnapshotUpdated(const AppUiSnapshot& snapshot)
{
    view.updateSnapshot(snapshot);
}
