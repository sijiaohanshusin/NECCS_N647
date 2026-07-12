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
    static const uint32_t MicCount = 32U;
    static const uint32_t PerfCount = 5U;
    static const uint32_t SysInfoCount = 10U;
    static const uint32_t ProfileCount = 3U;
    static const uint32_t ParamRowCount = 5U;
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
    void onSystemPressed(const touchgfx::AbstractButton& source);
    void setViewerOpen(bool open);
    void onMenuPressed(const touchgfx::AbstractButton& source);
    void onBandChanged(uint16_t loHz, uint16_t hiHz);
    void setMenuOpen(bool open);
    /* Invalidate only the ring outline strips of a boot ring bounding box,
     * keeping the emblem region untouched (prevents logo flicker). */
    void invalidateRingBand(int16_t x, int16_t y, int16_t diameter);

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
    AppRoundedPanel alertBanner;    /* mic-array reconnect / failure toast */
    AppTextLabel alertLabel;

    /* ---- popup navigation menu (opens from the brand area) ---- */
    touchgfx::TouchArea menuTouch;
    touchgfx::Box menuBurger[3];
    touchgfx::Box menuScrim;        /* full-screen dim + outside-tap dismiss */
    touchgfx::TouchArea menuScrimTouch;
    AppRoundedPanel menuPanel;
    touchgfx::Image navIcon[NavCount];
    AppTextLabel navLabel[NavCount];
    touchgfx::Box navActiveDot[NavCount];
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
    AppSpectrumPanel spectrumPanel;

    /* AI acoustic-signature card (rail bottom) */
    AppRoundedPanel aiCard;
    AppTextLabel aiTitle;
    AppTextLabel aiClassLabel;
    AppTextLabel aiConfLabel;
    touchgfx::Box aiConfTrack;
    touchgfx::Box aiConfFill;
    AppTextLabel spectrumBandLabel;

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
    AppTextLabel sysAccelLabel;
    /* power controls: reboot is immediate, power-off needs a confirm tap */
    AppRoundedPanel sysRebootBtn;
    AppTextLabel sysRebootLabel;
    touchgfx::TouchArea sysRebootTouch;
    AppRoundedPanel sysPowerBtn;
    AppTextLabel sysPowerLabel;
    touchgfx::TouchArea sysPowerTouch;

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
    /* render-parameter steppers: 4 rows x (-, +) */
    static const uint32_t RenderRowCount = 4U;
    AppTextLabel paramsRenderCaption;
    AppRoundedPanel renderRowPanel[RenderRowCount];
    AppTextLabel renderRowName[RenderRowCount];
    AppTextLabel renderRowValue[RenderRowCount];
    AppRoundedPanel renderStepChip[RenderRowCount * 2U];
    AppTextLabel renderStepLabel[RenderRowCount * 2U];
    touchgfx::TouchArea renderStepTouch[RenderRowCount * 2U];

    /* ---- media page (gallery grid + fullscreen viewer overlay) ---- */
    static const uint32_t MediaThumbSlots = 8U;
    AppTextLabel mediaTitle;
    AppTextLabel mediaCountLabel;
    AppRgb565Preview mediaThumb[MediaThumbSlots];
    AppTextLabel mediaThumbLabel[MediaThumbSlots];
    touchgfx::Box mediaThumbBadge[MediaThumbSlots];  /* video marker strip */
    touchgfx::TouchArea mediaThumbTouch[MediaThumbSlots];
    AppRoundedPanel mediaPageBtn[2];
    AppTextLabel mediaPageBtnLabel[2];
    touchgfx::TouchArea mediaPageTouch[2];
    AppTextLabel mediaEmptyLabel;
    AppRoundedPanel mediaButton[MediaActionCount];
    touchgfx::Image mediaButtonIcon[MediaActionCount];
    AppTextLabel mediaButtonLabel[MediaActionCount];
    touchgfx::TouchArea mediaTouch[MediaActionCount];
    /* viewer overlay */
    touchgfx::Box viewerScrim;
    AppRgb565Preview mediaPreview;
    AppTextLabel viewerCaption;
    touchgfx::TouchArea viewerTouch;

    /* ---- boot page ---- */
    static const uint32_t BootRingCount = 3U;
    touchgfx::Box bootBg;
    touchgfx::Image bootDecoWave;   /* enclosure silkscreen: gold wave */
    touchgfx::Image sysDecoSonar;   /* enclosure silkscreen: sonar arcs */
    touchgfx::Image bootCompBadge;  /* competition badge (9th AI design) */
    touchgfx::ScalableImage bootRing[BootRingCount];
    touchgfx::Image bootEmblem;
    AppTextLabel bootTitle;
    AppTextLabel bootSubtitle;
    touchgfx::Box bootItemDot[BootItemCount];
    AppTextLabel bootItemName[BootItemCount];
    AppTextLabel bootItemState[BootItemCount];
    touchgfx::Box bootBarTrack;
    touchgfx::Box bootBarFill;
    touchgfx::Box bootOrbitDot;     /* small orbiting dot = the only boot anim */
    AppTextLabel bootVersion;

    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> navPressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> quickPressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> profilePressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> paramsPressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> mediaPressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> menuPressedCallback;
    touchgfx::Callback<TemplateView, const touchgfx::AbstractButton&> systemPressedCallback;
    touchgfx::Callback<TemplateView, uint16_t, uint16_t> bandChangedCallback;

    uint8_t activeScreen;
    uint8_t activeProfile;
    uint8_t bootEmblemAlpha;
    uint16_t bootPhase;
    int16_t bootBarWidth;
    int16_t bootBarTarget;
    uint32_t mediaPreviewGeneration;
    uint32_t mediaThumbGeneration;
    uint32_t thumbPage;
    uint32_t thumbPageCount;
    uint8_t thumbTypes[MediaThumbSlots];
    bool recActive;
    bool menuOpen;
    bool viewerOpen;
    uint8_t selectedSlot;
    /* nonzero = power-off confirm window is open (ticks remaining) */
    uint8_t powerConfirmTicks;
};

#endif // TEMPLATE_VIEW_HPP
