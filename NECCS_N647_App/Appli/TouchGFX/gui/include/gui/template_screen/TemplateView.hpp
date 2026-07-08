/******************************************************************************
* NECCS N647 acoustic camera UI.
*
* Single TouchGFX screen hosting the boot/self-check page and the five main
* pages (imaging / array / params / media / system). All layout is hand-coded;
* the Model pushes an AppUiSnapshot every few ticks and the View renders it.
*******************************************************************************/

#ifndef TEMPLATE_VIEW_HPP
#define TEMPLATE_VIEW_HPP

#include <mvp/View.hpp>
#include <gui/common/AppUiWidgets.hpp>
#include <gui/model/Model.hpp>
#include <gui/template_screen/TemplatePresenter.hpp>
#include <touchgfx/Callback.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/Image.hpp>
#include <touchgfx/widgets/ScalableImage.hpp>
#include <touchgfx/widgets/TouchArea.hpp>

class TemplateView : public touchgfx::View<TemplatePresenter>
{
public:
    TemplateView();

    virtual ~TemplateView()
    {
    }

    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();

    void updateSnapshot(const AppUiSnapshot& snapshot);

private:
    static const uint32_t NavCount = 5U;
    static const uint32_t QuickCount = 5U;
    static const uint32_t RailCardCount = 4U;
    static const uint32_t RailStateRows = 3U;
    static const uint32_t StripCount = 4U;
    static const uint32_t MicCount = 32U;
    static const uint32_t PerfCount = 5U;
    static const uint32_t SysInfoCount = 10U;
    static const uint32_t ProfileCount = 3U;
    static const uint32_t ParamRowCount = 5U;
    static const uint32_t MediaInfoCount = 8U;
    static const uint32_t MediaActionCount = 5U;
    static const uint32_t BootItemCount = 5U;

    void setupStatusBar();
    void setupNavigation();
    void setupImagePage();
    void setupMicPage();
    void setupSystemPage();
    void setupParamsPage();
    void setupMediaPage();
    void setupBootPage();

    void refreshVisibility();
    void refreshNavigation();
    void refreshStatusBar(const AppUiSnapshot& snapshot);
    void refreshBootPage(const AppUiSnapshot& snapshot);
    void refreshImagePage(const AppUiSnapshot& snapshot);
    void refreshMicPage(const AppUiSnapshot& snapshot);
    void refreshSystemPage(const AppUiSnapshot& snapshot);
    void refreshParamsPage(const AppUiSnapshot& snapshot);
    void refreshMediaPage(const AppUiSnapshot& snapshot);

    void onNavPressed(const touchgfx::AbstractButton& source);
    void onQuickPressed(const touchgfx::AbstractButton& source);
    void onProfilePressed(const touchgfx::AbstractButton& source);
    void onParamsPressed(const touchgfx::AbstractButton& source);
    void onMediaPressed(const touchgfx::AbstractButton& source);

    /* ---- chrome ---- */
    touchgfx::Box background;
    touchgfx::Box cameraPreviewKey;
    touchgfx::Box topBar;
    touchgfx::Box topBarLine;
    touchgfx::Image brandMark;
    AppTextLabel brandTitle;
    AppRoundedPanel modeChip;
    AppTextLabel modeChipLabel;
    touchgfx::Box recDot;
    AppTextLabel recLabel;
    AppTextLabel sdLabel;
    AppTextLabel battLabel;
    AppTextLabel fpsLabel;

    /* ---- navigation rail ---- */
    touchgfx::Box navPanel;
    touchgfx::Box navPanelLine;
    touchgfx::Box navActiveBar;
    touchgfx::Image navIcon[NavCount];
    AppTextLabel navLabel[NavCount];
    touchgfx::TouchArea navTouch[NavCount];

    /* ---- imaging page ---- */
    touchgfx::Box cameraFrame[4];
    touchgfx::Box cameraCorner[8];
    AppRoundedPanel quickButton[QuickCount];
    touchgfx::Image quickIcon[QuickCount];
    AppTextLabel quickLabel[QuickCount];
    touchgfx::TouchArea quickTouch[QuickCount];
    AppRoundedPanel railCard[RailCardCount];
    AppTextLabel railSourceTitle;
    AppTextLabel railTheta;
    AppTextLabel railPhi;
    touchgfx::Box railQualityTrack;
    touchgfx::Box railQualityFill;
    AppTextLabel railStateTitle;
    AppTextLabel railStateName[RailStateRows];
    AppTextLabel railStateValue[RailStateRows];
    AppTextLabel railPerfTitle;
    AppTextLabel railPerfMs;
    AppTextLabel railPerfFps;
    AppTextLabel railPerfCam;
    AppTextLabel railModeTitle;
    AppTextLabel railModeValue;
    AppTextLabel railSceneValue;
    AppTextLabel railCandLabel[2];
    touchgfx::Box stripPanel;
    AppTextLabel stripLabel[StripCount];

    /* ---- array page ---- */
    AppTextLabel micTitle;
    AppRoundedPanel micCell[MicCount];
    AppTextLabel micIndexLabel[MicCount];
    AppTextLabel micDbLabel[MicCount];
    AppTextLabel micSummary[4];

    /* ---- system page ---- */
    AppTextLabel sysTitle;
    AppRoundedPanel sysPerfCard;
    AppRoundedPanel sysInfoCard;
    AppTextLabel perfName[PerfCount];
    touchgfx::Box perfTrack[PerfCount];
    touchgfx::Box perfFill[PerfCount];
    AppTextLabel perfValue[PerfCount];
    AppTextLabel sysInfoName[SysInfoCount];
    AppTextLabel sysInfoValue[SysInfoCount];

    /* ---- params page ---- */
    AppTextLabel paramsTitle;
    AppTextLabel paramsProfileCaption;
    AppRoundedPanel profileChip[ProfileCount];
    AppTextLabel profileChipLabel[ProfileCount];
    touchgfx::TouchArea profileTouch[ProfileCount];
    AppRoundedPanel paramRowPanel[ParamRowCount];
    AppTextLabel paramRowName[ParamRowCount];
    AppTextLabel paramRowValue[ParamRowCount];
    touchgfx::TouchArea paramRowTouch[ParamRowCount];
    AppRoundedPanel tempStepChip[2];
    AppTextLabel tempStepLabel[2];
    touchgfx::TouchArea tempStepTouch[2];

    /* ---- media page ---- */
    AppTextLabel mediaTitle;
    AppRgb565Preview mediaPreview;
    AppRoundedPanel mediaInfoCard;
    AppTextLabel mediaInfoName[MediaInfoCount];
    AppTextLabel mediaInfoValue[MediaInfoCount];
    AppRoundedPanel mediaButton[MediaActionCount];
    touchgfx::Image mediaButtonIcon[MediaActionCount];
    AppTextLabel mediaButtonLabel[MediaActionCount];
    touchgfx::TouchArea mediaTouch[MediaActionCount];

    /* ---- boot page ---- */
    static const uint32_t BootRingCount = 3U;
    touchgfx::Box bootBg;
    touchgfx::ScalableImage bootRing[BootRingCount];
    touchgfx::Image bootEmblem;
    AppTextLabel bootTitle;
    AppTextLabel bootSubtitle;
    touchgfx::Box bootItemDot[BootItemCount];
    AppTextLabel bootItemName[BootItemCount];
    AppTextLabel bootItemState[BootItemCount];
    touchgfx::Box bootBarTrack;
    touchgfx::Box bootBarFill;
    AppTextLabel bootVersion;

    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> navPressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> quickPressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> profilePressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> paramsPressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> mediaPressedCallback;

    uint8_t activeScreen;
    uint8_t activeProfile;
    uint8_t bootEmblemAlpha;
    uint16_t bootPhase;
    int16_t bootBarWidth;
    int16_t bootBarTarget;
    uint32_t mediaPreviewGeneration;
    bool recActive;
};

#endif // TEMPLATE_VIEW_HPP
