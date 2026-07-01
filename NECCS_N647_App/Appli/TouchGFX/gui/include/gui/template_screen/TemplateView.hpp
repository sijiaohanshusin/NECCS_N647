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

#ifndef TEMPLATE_VIEW_HPP
#define TEMPLATE_VIEW_HPP

#include <mvp/View.hpp>
#include <gui/common/AppUiWidgets.hpp>
#include <gui/model/Model.hpp>
#include <gui/template_screen/TemplatePresenter.hpp>
#include <touchgfx/Callback.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/BoxWithBorder.hpp>
#include <touchgfx/widgets/TouchArea.hpp>

/**
 * The View for a template screen. In the MVP paradigm, the view is a
 * passive interface that only displays the user interface and routes
 * any events from the user interface to the presenter. Normally it is the
 * responsibility of the presenter to act upon those events.
 *
 * By deriving from View, this class has a presenter pointer, which is configured
 * automatically.
 */
class TemplateView : public touchgfx::View<TemplatePresenter>
{
public:
    /**
     * The view constructor. Note that the constructor runs before the new screen is
     * completely initialized, so do not place code here that causes anything to be drawn.
     * Use the setupScreen function instead.
     */
    TemplateView();

    virtual ~TemplateView()
    {
    }

    /**
     * This function is called automatically when the view is activated. This function
     * should add widgets to the root container, configure widget sizes and event
     * callbacks, et cetera.
     */
    virtual void setupScreen();

    /**
     * This function is called automatically when transitioning to a different screen
     * than this one. Can optionally be used to clean up.
     */
    virtual void tearDownScreen();

    void updateSnapshot(const AppUiSnapshot& snapshot);

private:
    static const uint32_t NavCount = 5U;
    static const uint32_t ProfileCount = 3U;
    static const uint32_t MicCount = 32U;
    static const uint32_t PerfCount = 5U;
    static const uint32_t DetailCount = 11U;
    static const uint32_t SettingsCount = 13U;
    static const uint32_t BqModeButtonCount = 2U;
    static const uint32_t MediaLabelCount = 10U;
    static const uint32_t MediaActionCount = 5U;

    void setupStaticUi();
    void setupNavigation();
    void setupImagePage();
    void setupMicPage();
    void setupPerfPage();
    void setupSettingsPage();
    void setupMediaPage();
    void setupDetails();
    void refreshVisibility();
    void refreshNavigation();
    void refreshProfileButtons();
    void refreshDetails(const AppUiSnapshot& snapshot);
    void refreshImagePage(const AppUiSnapshot& snapshot);
    void refreshMicPage(const AppUiSnapshot& snapshot);
    void refreshPerfPage(const AppUiSnapshot& snapshot);
    void refreshSettingsPage(const AppUiSnapshot& snapshot);
    void refreshMediaPage(const AppUiSnapshot& snapshot);
    void onNavPressed(const touchgfx::AbstractButton& source);
    void onProfilePressed(const touchgfx::AbstractButton& source);
    void onMediaPressed(const touchgfx::AbstractButton& source);
    void onBqModePressed(const touchgfx::AbstractButton& source);

    touchgfx::Box background;
    touchgfx::Box topBar;
    touchgfx::Box navPanel;
    touchgfx::Box contentPanel;
    touchgfx::Box detailPanel;
    touchgfx::Box statusDot;
    touchgfx::BoxWithBorder navButton[NavCount];
    touchgfx::TouchArea navTouch[NavCount];
    AppTextLabel navLabel[NavCount];

    touchgfx::BoxWithBorder profileButton[ProfileCount];
    touchgfx::TouchArea profileTouch[ProfileCount];
    AppTextLabel profileLabel[ProfileCount];
    touchgfx::BoxWithBorder bqModeButton[BqModeButtonCount];
    touchgfx::TouchArea bqModeTouch[BqModeButtonCount];
    AppTextLabel bqModeLabel[BqModeButtonCount];

    AppTextLabel titleLabel;
    AppTextLabel modeLabel;
    AppTextLabel pageTitleLabel;
    AppTextLabel detailLabel[DetailCount];
    AppTextLabel settingsLabel[SettingsCount];
    AppTextLabel mediaLabel[MediaLabelCount];

    AppHeatMap heatMap;
    AppTextLabel heatMetricLabel[4];

    touchgfx::BoxWithBorder micCell[MicCount];
    AppTextLabel micSummaryLabel[4];

    touchgfx::Box perfTrack[PerfCount];
    touchgfx::Box perfFill[PerfCount];
    AppTextLabel perfLabel[PerfCount];

    touchgfx::BoxWithBorder mediaButton[MediaActionCount];
    touchgfx::TouchArea mediaTouch[MediaActionCount];
    AppTextLabel mediaButtonLabel[MediaActionCount];

    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> navPressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> profilePressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> mediaPressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> bqModePressedCallback;

    uint8_t activeScreen;
    uint8_t activeProfile;
};

#endif // TEMPLATE_VIEW_HPP
