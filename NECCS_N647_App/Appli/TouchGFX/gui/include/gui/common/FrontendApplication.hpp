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

#ifndef FRONTENDAPPLICATION_HPP
#define FRONTENDAPPLICATION_HPP

#include <mvp/MVPApplication.hpp>
#include <mvp/View.hpp>
#include <gui/model/Model.hpp>

class FrontendHeap;

/**
 * The FrontendApplication is the concrete implementation of an MVPApplication
 * for a specific application. Its primary job is to provide functions for
 * switching between screens (presenter/view pairs). By convention these functions
 * are usually called gotoXXScreen().
 *
 * Note that the base class Application is a singleton. Switching screen can be done
 * from anywhere (usually presenters) by e.g. the following:
 * static_cast<FrontendApplication*>(Application::getInstance())->gotoXXScreen();
 */
class FrontendApplication : public touchgfx::MVPApplication
{
public:
    FrontendApplication(Model& m, FrontendHeap& heap);

    virtual ~FrontendApplication()
    {
    }

    virtual void changeToStartScreen()
    {
        gotoTemplateScreen();
    }

    /**
     * Request a transition to the "Template" screen. The actual switching will
     * be done at next tick (by gotoTemplateScreenImpl).
     *
     * @note The reason for not switching immediately is that the new presenter and view
     * reuse the same memory as the current ones, so the switch cannot occur instantaneously
     * since that would cause memory corruption at the point of calling this function.
     */
    void gotoTemplateScreen();

    /**
     * Called automatically every frame. Will call tick on the model and then delegate
     * the tick event to the framework for further processing.
     */
    virtual void handleTickEvent()
    {
        model.tick();
        touchgfx::MVPApplication::handleTickEvent();
    }

private:
    touchgfx::Callback<FrontendApplication> transitionCallback;
    FrontendHeap& frontendHeap;
    Model& model;
    void gotoTemplateScreenImpl();
};

#endif /* FRONTENDAPPLICATION_HPP */
