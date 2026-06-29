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

#include <new>
#include <mvp/View.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/lcd/LCD.hpp>
#include <touchgfx/transitions/NoTransition.hpp>
#include <gui/common/FrontendApplication.hpp>
#include <gui/common/FrontendHeap.hpp>
#include <gui/template_screen/TemplatePresenter.hpp>
#include <gui/template_screen/TemplateView.hpp>

FrontendApplication::FrontendApplication(Model& m, FrontendHeap& heap)
    : touchgfx::MVPApplication(),
      transitionCallback(),
      frontendHeap(heap),
      model(m)
{
}

void FrontendApplication::gotoTemplateScreen()
{
    transitionCallback = touchgfx::Callback<FrontendApplication>(this, &FrontendApplication::gotoTemplateScreenImpl);
    pendingScreenTransitionCallback = &transitionCallback;
}

void FrontendApplication::gotoTemplateScreenImpl()
{
    touchgfx::makeTransition<TemplateView, TemplatePresenter, touchgfx::NoTransition, Model>(&currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}
