#include <gui/template_screen/TemplateView.hpp>

#include <images/BitmapDatabase.hpp>
#include <touchgfx/Application.hpp>
#include <touchgfx/Color.hpp>

#include <math.h>
#include <stdio.h>

namespace
{
/* ---- geometry ---- */
constexpr int16_t ScreenW = 1024;
constexpr int16_t ScreenH = 600;
constexpr int16_t BarH = 44;
constexpr int16_t NavW = 76;
constexpr int16_t CamX = 192;
constexpr int16_t CamY = 60;
constexpr int16_t CamW = 640;
constexpr int16_t CamH = 480;
constexpr int16_t ContentX = 92;
constexpr int16_t RailX = 844;
constexpr int16_t RailW = 172;

/* ---- design tokens ---- */
touchgfx::colortype rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return touchgfx::Color::getColorFromRGB(r, g, b);
}

const touchgfx::colortype ColorBg = rgb(7, 13, 20);
const touchgfx::colortype ColorPanel = rgb(15, 23, 35);
const touchgfx::colortype ColorPanel2 = rgb(22, 32, 46);
const touchgfx::colortype ColorPanelHi = rgb(31, 44, 62);
const touchgfx::colortype ColorLine = rgb(36, 53, 74);
const touchgfx::colortype ColorBlue = rgb(61, 126, 255);
const touchgfx::colortype ColorBlueDim = rgb(28, 58, 110);
const touchgfx::colortype ColorRed = rgb(229, 72, 77);
const touchgfx::colortype ColorRedDim = rgb(96, 34, 40);
const touchgfx::colortype ColorGreen = rgb(46, 194, 126);
const touchgfx::colortype ColorAmber = rgb(245, 184, 74);
const touchgfx::colortype ColorText = rgb(242, 245, 249);
const touchgfx::colortype ColorMuted = rgb(140, 160, 180);

const char* profileName(uint8_t profile)
{
    switch (profile)
    {
    case APP_UI_PROFILE_FAST:
        return "快速";
    case APP_UI_PROFILE_QUALITY:
        return "质量";
    default:
        return "标准";
    }
}

const char* sceneName(uint8_t scene)
{
    switch (scene)
    {
    case 1U:
        return "气体泄漏";
    case 2U:
        return "轴承";
    case 3U:
        return "电气";
    default:
        return "通用";
    }
}

const char* paletteName(uint8_t palette)
{
    switch (palette)
    {
    case 1U:
        return "彩虹";
    case 2U:
        return "高对比";
    default:
        return "铁红";
    }
}

const char* powerStateName(uint8_t state)
{
    switch (state)
    {
    case APP_UI_POWER_STATE_IDLE:
        return "待机";
    case APP_UI_POWER_STATE_CHARGING:
        return "充电";
    case APP_UI_POWER_STATE_DISCHARGING:
        return "放电";
    case APP_UI_POWER_STATE_OTG:
        return "OTG";
    case APP_UI_POWER_STATE_UNDERVOLTAGE:
        return "低压";
    case APP_UI_POWER_STATE_FAULT:
        return "故障";
    default:
        return "USB";
    }
}

touchgfx::colortype levelColor(uint8_t level)
{
    if (level > 82U)
    {
        return rgb(120, 45, 48);
    }
    if (level > 58U)
    {
        return rgb(112, 84, 34);
    }
    if (level > 28U)
    {
        return rgb(28, 84, 66);
    }
    return rgb(24, 36, 52);
}

void setupLabel(AppTextLabel& label,
                int16_t x,
                int16_t y,
                int16_t w,
                int16_t h,
                uint8_t scale,
                const char* text,
                touchgfx::colortype fg,
                AppTextLabel::Align align = AppTextLabel::ALIGN_LEFT)
{
    label.setPosition(x, y, w, h);
    label.setScale(scale);
    label.setColors(fg, ColorBg, false);
    label.setAlignment(align);
    label.setText(text);
}

void formatCyclesM(char* buffer, uint32_t bufferSize, uint32_t cycles)
{
    (void)snprintf(buffer,
                   bufferSize,
                   "%lu.%luM",
                   static_cast<unsigned long>(cycles / 1000000UL),
                   static_cast<unsigned long>((cycles / 100000UL) % 10UL));
}
}

TemplateView::TemplateView()
    : navPressedCallback(this, &TemplateView::onNavPressed),
      quickPressedCallback(this, &TemplateView::onQuickPressed),
      profilePressedCallback(this, &TemplateView::onProfilePressed),
      paramsPressedCallback(this, &TemplateView::onParamsPressed),
      mediaPressedCallback(this, &TemplateView::onMediaPressed),
      menuPressedCallback(this, &TemplateView::onMenuPressed),
      systemPressedCallback(this, &TemplateView::onSystemPressed),
      bandChangedCallback(this, &TemplateView::onBandChanged),
      beamPressedCallback(this, &TemplateView::onBeamPressed),
      beamAimCallback(this, &TemplateView::onBeamAim),
      activeScreen(APP_UI_SCREEN_BOOT),
      activeProfile(APP_UI_PROFILE_BALANCED),
      bootEmblemAlpha(0U),
      bootPhase(0U),
      bootBarWidth(0),
      bootBarTarget(0),
      mediaPreviewGeneration(0U),
      mediaThumbGeneration(0U),
      thumbPage(0U),
      thumbPageCount(1U),
      recActive(false),
      menuOpen(false),
      viewerOpen(false),
      beamUiRecording(false),
      selectedSlot(0U),
      powerConfirmTicks(0U)
{
}

void TemplateView::setupScreen()
{
    background.setPosition(0, 0, ScreenW, ScreenH);
    background.setColor(ColorBg);
    add(background);

    cameraPreviewKey.setPosition(CamX, CamY, CamW, CamH);
    cameraPreviewKey.setColor(rgb(255, 0, 255));
    cameraPreviewKey.setVisible(false);
    add(cameraPreviewKey);

    setupImagePage();
    setupBeamPage();
    setupMicPage();
    setupSystemPage();
    setupParamsPage();
    setupMediaPage();
    setupStatusBar();
    setupBootPage();

    /* Mic-array reconnect toast: overlays pages (but sits under the menu). */
    alertBanner.setPosition(312, 52, 400, 36);
    alertBanner.setStyle(rgb(64, 34, 20), 18U);
    alertBanner.setBorder(ColorAmber, true);
    alertBanner.setVisible(false);
    add(alertBanner);
    setupLabel(alertLabel, 312, 60, 400, 22, 1, "", ColorAmber, AppTextLabel::ALIGN_CENTER);
    alertLabel.setVisible(false);
    add(alertLabel);

    /* Menu popup last: it overlays everything. */
    setupNavigation();

    refreshVisibility();
    refreshNavigation();
    invalidate();
}

void TemplateView::tearDownScreen()
{
}

/* ------------------------------------------------------------------ */
/* setup                                                               */
/* ------------------------------------------------------------------ */

void TemplateView::setupStatusBar()
{
    topBar.setPosition(0, 0, ScreenW, BarH);
    topBar.setColor(ColorPanel);
    add(topBar);

    topBarLine.setPosition(0, BarH, ScreenW, 1);
    topBarLine.setColor(ColorLine);
    add(topBarLine);

    brandMark.setBitmap(touchgfx::Bitmap(BITMAP_BRAND_MARK_ID));
    brandMark.setPosition(14, 6, 35, 32);
    add(brandMark);

    setupLabel(brandTitle, 58, 10, 170, 26, 2, "声学成像仪", ColorText);
    add(brandTitle);

    /* Brand area doubles as the menu button (hamburger glyph, no font dep). */
    for (uint32_t i = 0U; i < 3U; ++i)
    {
        menuBurger[i].setPosition(216, static_cast<int16_t>(15 + (i * 5)), 18, 2);
        menuBurger[i].setColor(ColorMuted);
        add(menuBurger[i]);
    }

    menuTouch.setPosition(0, 0, 244, BarH);
    menuTouch.setAction(menuPressedCallback);
    add(menuTouch);

    modeChip.setPosition(250, 8, 190, 28);
    modeChip.setStyle(ColorPanel2, 14U);
    modeChip.setBorder(ColorBlueDim, true);
    add(modeChip);

    setupLabel(modeChipLabel, 250, 12, 190, 20, 1, "通用 · 标准", ColorBlue, AppTextLabel::ALIGN_CENTER);
    add(modeChipLabel);

    recDot.setPosition(786, 17, 10, 10);
    recDot.setColor(ColorRed);
    recDot.setVisible(false);
    add(recDot);

    /* Blue tick while the beamformed WAV is being written (the red video
     * REC dot may be active at the same time). Sits in the 770..786 gap
     * between the SD label text and the REC dot. */
    beamDot.setPosition(773, 17, 10, 10);
    beamDot.setColor(ColorBlue);
    beamDot.setVisible(false);
    add(beamDot);

    setupLabel(recLabel, 802, 11, 60, 22, 1, "", ColorRed);
    recLabel.setVisible(false);
    add(recLabel);

    setupLabel(sdLabel, 660, 11, 110, 22, 1, "SD --", ColorMuted, AppTextLabel::ALIGN_RIGHT);
    add(sdLabel);

    setupLabel(battLabel, 500, 11, 110, 22, 1, "", ColorMuted, AppTextLabel::ALIGN_RIGHT);
    battLabel.setVisible(false);
    add(battLabel);

    setupLabel(fpsLabel, 902, 11, 110, 22, 1, "--", ColorMuted, AppTextLabel::ALIGN_RIGHT);
    add(fpsLabel);
}

void TemplateView::setupNavigation()
{
    /* Popup menu: dim scrim (tap outside to close) + panel under the brand
     * area with one row per page. Added last, so it overlays every page. */
    /* Fully opaque scrim: a semi-transparent one blends with the LTDC
     * color-key magenta inside the camera hole, producing pixels that no
     * longer match the key - the panel then shows a purple rectangle.
     * Opaque guarantees the hole is covered whatever the key state is. */
    menuScrim.setPosition(0, 0, ScreenW, ScreenH);
    menuScrim.setColor(rgb(5, 9, 15));
    menuScrim.setAlpha(255U);
    add(menuScrim);

    menuScrimTouch.setPosition(0, 0, ScreenW, ScreenH);
    menuScrimTouch.setAction(menuPressedCallback);
    add(menuScrimTouch);

    menuPanel.setPosition(10, BarH + 6, 250, 6 * 62 + 20);
    menuPanel.setStyle(ColorPanel2, 14U);
    menuPanel.setBorder(ColorBlueDim, true);
    add(menuPanel);

    static const char* labels[NavCount] = {"实时成像", "声源录音", "麦克风阵列", "参数设置", "媒体中心", "系统状态"};
    static const uint16_t icons[NavCount] = {
        BITMAP_UI_IMAGE_ID,
        BITMAP_UI_MIC_ID,
        BITMAP_UI_MIC_ID,
        BITMAP_UI_SETTINGS_ID,
        BITMAP_UI_MEDIA_ID,
        BITMAP_UI_PERF_ID
    };

    for (uint32_t i = 0U; i < NavCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(BarH + 16 + (i * 62));

        navActiveDot[i].setPosition(26, static_cast<int16_t>(y + 20), 6, 6);
        navActiveDot[i].setColor(ColorBlue);
        add(navActiveDot[i]);

        navIcon[i].setBitmap(touchgfx::Bitmap(icons[i]));
        navIcon[i].setPosition(44, static_cast<int16_t>(y + 10), 24, 24);
        add(navIcon[i]);

        setupLabel(navLabel[i], 82, static_cast<int16_t>(y + 11), 160, 24, 2, labels[i], ColorText);
        add(navLabel[i]);

        navTouch[i].setPosition(14, y, 242, 58);
        navTouch[i].setAction(navPressedCallback);
        add(navTouch[i]);
    }
}

void TemplateView::setMenuOpen(bool open)
{
    menuOpen = open;
    /* Hide the camera layer while the menu overlays it: the semi-transparent
     * scrim otherwise blends with the LTDC color-key hole and shows up as a
     * purple smear over the video area. */
    presenter->setMenuBlocksCamera(open);
    menuScrim.setVisible(open);
    menuScrimTouch.setVisible(open);
    menuPanel.setVisible(open);
    for (uint32_t i = 0U; i < NavCount; ++i)
    {
        navIcon[i].setVisible(open);
        navLabel[i].setVisible(open);
        navTouch[i].setVisible(open);
        navActiveDot[i].setVisible(open);
    }
    if (open)
    {
        refreshNavigation();
    }
    invalidate();
}

void TemplateView::setupImagePage()
{
    /* camera window frame + corner accents */
    cameraFrame[0].setPosition(CamX - 3, CamY - 3, CamW + 6, 2);
    cameraFrame[1].setPosition(CamX - 3, CamY + CamH + 1, CamW + 6, 2);
    cameraFrame[2].setPosition(CamX - 3, CamY - 3, 2, CamH + 6);
    cameraFrame[3].setPosition(CamX + CamW + 1, CamY - 3, 2, CamH + 6);
    for (uint32_t i = 0U; i < 4U; ++i)
    {
        cameraFrame[i].setColor(ColorLine);
        add(cameraFrame[i]);
    }

    const int16_t cornerLen = 22;
    const int16_t cx0 = CamX - 3;
    const int16_t cy0 = CamY - 3;
    const int16_t cx1 = CamX + CamW + 1;
    const int16_t cy1 = CamY + CamH + 1;
    cameraCorner[0].setPosition(cx0, cy0, cornerLen, 2);
    cameraCorner[1].setPosition(cx0, cy0, 2, cornerLen);
    cameraCorner[2].setPosition(static_cast<int16_t>(cx1 + 2 - cornerLen), cy0, cornerLen, 2);
    cameraCorner[3].setPosition(cx1, cy0, 2, cornerLen);
    cameraCorner[4].setPosition(cx0, cy1, cornerLen, 2);
    cameraCorner[5].setPosition(cx0, static_cast<int16_t>(cy1 + 2 - cornerLen), 2, cornerLen);
    cameraCorner[6].setPosition(static_cast<int16_t>(cx1 + 2 - cornerLen), cy1, cornerLen, 2);
    cameraCorner[7].setPosition(cx1, static_cast<int16_t>(cy1 + 2 - cornerLen), 2, cornerLen);
    for (uint32_t i = 0U; i < 8U; ++i)
    {
        cameraCorner[i].setColor(ColorBlue);
        add(cameraCorner[i]);
    }

    /* left quick-action column */
    static const char* quickNames[QuickCount] = {"截屏", "录像", "触发", "调色板", "模式"};
    static const uint16_t quickIcons[QuickCount] = {
        BITMAP_UI_SNAPSHOT_ID,
        BITMAP_UI_RECORD_ID,
        BITMAP_UI_PEAK_ID,
        BITMAP_UI_QUALITY_ID,
        BITMAP_UI_STANDARD_ID
    };
    /* Quick actions fill the left column freed by the popup navigation. */
    for (uint32_t i = 0U; i < QuickCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(CamY + (i * 86));
        quickButton[i].setPosition(16, y, 160, 78);
        quickButton[i].setStyle(ColorPanel2, 12U);
        quickButton[i].setBorder(ColorLine, true);
        add(quickButton[i]);

        quickIcon[i].setBitmap(touchgfx::Bitmap(quickIcons[i]));
        quickIcon[i].setPosition(84, static_cast<int16_t>(y + 12), 24, 24);
        add(quickIcon[i]);

        setupLabel(quickLabel[i], 16, static_cast<int16_t>(y + 46), 160, 20, 1, quickNames[i], ColorText, AppTextLabel::ALIGN_CENTER);
        add(quickLabel[i]);

        quickTouch[i].setPosition(16, y, 160, 78);
        quickTouch[i].setAction(quickPressedCallback);
        add(quickTouch[i]);
    }

    /* right data rail */
    railCard[0].setPosition(RailX, 60, RailW, 132);
    railCard[1].setPosition(RailX, 200, RailW, 104);
    railCard[2].setPosition(RailX, 312, RailW, 104);
    railCard[3].setPosition(RailX, 424, RailW, 84);
    for (uint32_t i = 0U; i < RailCardCount; ++i)
    {
        railCard[i].setStyle(ColorPanel, 10U);
        railCard[i].setBorder(ColorLine, true);
        add(railCard[i]);
    }

    setupLabel(railSourceTitle, RailX + 14, 68, 144, 18, 1, "声源方位", ColorMuted);
    setupLabel(railTheta, RailX + 14, 88, 144, 24, 2, "-- °", ColorText);
    setupLabel(railPhi, RailX + 14, 112, 144, 24, 2, "-- °", ColorText);
    add(railSourceTitle);
    add(railTheta);
    add(railPhi);

    railQualityTrack.setPosition(RailX + 14, 142, 144, 5);
    railQualityTrack.setColor(ColorPanel2);
    add(railQualityTrack);
    railQualityFill.setPosition(RailX + 14, 142, 4, 5);
    railQualityFill.setColor(ColorBlue);
    add(railQualityFill);

    /* Secondary source rows (multi-source list). */
    for (uint32_t i = 0U; i < 2U; ++i)
    {
        setupLabel(railCandLabel[i],
                   RailX + 14,
                   static_cast<int16_t>(152 + (i * 18)),
                   144,
                   17,
                   1,
                   "",
                   rgb(120, 214, 255));
        add(railCandLabel[i]);
    }

    setupLabel(railStateTitle, RailX + 14, 208, 144, 18, 1, "链路状态", ColorMuted);
    add(railStateTitle);
    static const char* stateNames[RailStateRows] = {"麦阵", "相机", "SD"};
    for (uint32_t i = 0U; i < RailStateRows; ++i)
    {
        const int16_t y = static_cast<int16_t>(230 + (i * 23));
        setupLabel(railStateName[i], RailX + 14, y, 60, 20, 1, stateNames[i], ColorMuted);
        setupLabel(railStateValue[i], RailX + 70, y, 88, 20, 1, "等待", ColorAmber, AppTextLabel::ALIGN_RIGHT);
        add(railStateName[i]);
        add(railStateValue[i]);
    }

    setupLabel(railPerfTitle, RailX + 14, 320, 144, 18, 1, "性能", ColorMuted);
    setupLabel(railPerfMs, RailX + 14, 342, 144, 24, 2, "SRP --", ColorText);
    setupLabel(railPerfFps, RailX + 14, 370, 144, 18, 1, "热图 --", ColorMuted);
    setupLabel(railPerfCam, RailX + 14, 390, 144, 18, 1, "相机 --", ColorMuted);
    add(railPerfTitle);
    add(railPerfMs);
    add(railPerfFps);
    add(railPerfCam);

    setupLabel(railModeTitle, RailX + 14, 432, 144, 18, 1, "Wide32 · 48k", ColorMuted);
    setupLabel(railModeValue, RailX + 14, 452, 144, 24, 2, "标准", ColorBlue);
    setupLabel(railSceneValue, RailX + 14, 478, 144, 18, 1, "通用场景", ColorMuted);
    add(railModeTitle);
    add(railModeValue);
    add(railSceneValue);

    /* AI acoustic-signature card */
    aiCard.setPosition(RailX, 516, RailW, 76);
    aiCard.setStyle(ColorPanel, 10U);
    aiCard.setBorder(ColorBlueDim, true);
    add(aiCard);

    setupLabel(aiTitle, RailX + 14, 522, 144, 18, 1, "AI 声纹识别", ColorMuted);
    add(aiTitle);

    setupLabel(aiClassLabel, RailX + 14, 541, 144, 24, 2, "监听中", ColorMuted);
    add(aiClassLabel);

    aiConfTrack.setPosition(RailX + 14, 574, 100, 5);
    aiConfTrack.setColor(ColorPanel2);
    add(aiConfTrack);

    aiConfFill.setPosition(RailX + 14, 574, 2, 5);
    aiConfFill.setColor(ColorBlue);
    add(aiConfFill);

    setupLabel(aiConfLabel, RailX + 118, 568, 40, 18, 1, "", ColorMuted, AppTextLabel::ALIGN_RIGHT);
    add(aiConfLabel);

    /* Interactive FFT spectrum with band selection under the camera window. */
    spectrumPanel.setPosition(CamX, 546, CamW, 46);
    spectrumPanel.setBandChangedCallback(bandChangedCallback);
    add(spectrumPanel);

    setupLabel(spectrumBandLabel, 16, 548, 160, 20, 1, "频带 -- Hz", ColorMuted, AppTextLabel::ALIGN_CENTER);
    add(spectrumBandLabel);
}

/* Directional-recording station: dedicated page around the fixed camera
 * window (the LTDC layer/color-key hole cannot move, so the skeleton stays
 * "left column + camera + right rail + bottom strip"). All chrome sits
 * OUTSIDE the camera hole - semi-transparent panels over the color-key
 * area would blend into purple smears. */
void TemplateView::setupBeamPage()
{
    /* Camera window border (this page's own frame). */
    beamCamFrame[0].setPosition(CamX - 3, CamY - 3, CamW + 6, 2);
    beamCamFrame[1].setPosition(CamX - 3, CamY + CamH + 1, CamW + 6, 2);
    beamCamFrame[2].setPosition(CamX - 3, CamY - 3, 2, CamH + 6);
    beamCamFrame[3].setPosition(CamX + CamW + 1, CamY - 3, 2, CamH + 6);
    for (uint32_t i = 0U; i < 4U; ++i)
    {
        beamCamFrame[i].setColor(ColorLine);
        add(beamCamFrame[i]);
    }

    /* Aim surface: tap-to-jump / drag-to-follow steering. */
    beamAimSurface.setPosition(CamX, CamY, CamW, CamH);
    beamAimSurface.setAimCallback(beamAimCallback);
    beamAimSurface.setVisible(false);
    beamAimSurface.setTouchable(false);
    add(beamAimSurface);

    /* Left column: big record button, tracking toggle, clip counter. */
    beamRecBtn.setPosition(16, CamY, 160, 96);
    beamRecBtn.setStyle(ColorRedDim, 14U);
    beamRecBtn.setBorder(ColorRed, true);
    add(beamRecBtn);
    setupLabel(beamRecLabel, 16, CamY + 38, 160, 22, 2, "录制", ColorRed, AppTextLabel::ALIGN_CENTER);
    add(beamRecLabel);
    beamRecTouch.setPosition(16, CamY, 160, 96);
    beamRecTouch.setAction(beamPressedCallback);
    add(beamRecTouch);

    beamAutoBtn.setPosition(16, CamY + 108, 160, 64);
    beamAutoBtn.setStyle(ColorBlueDim, 12U);
    beamAutoBtn.setBorder(ColorBlue, true);
    add(beamAutoBtn);
    setupLabel(beamAutoLabel, 16, CamY + 130, 160, 20, 1, "自动", ColorBlue, AppTextLabel::ALIGN_CENTER);
    add(beamAutoLabel);
    beamAutoTouch.setPosition(16, CamY + 108, 160, 64);
    beamAutoTouch.setAction(beamPressedCallback);
    add(beamAutoTouch);

    beamClipsCard.setPosition(16, CamY + 184, 160, 96);
    beamClipsCard.setStyle(ColorPanel, 12U);
    beamClipsCard.setBorder(ColorLine, true);
    add(beamClipsCard);
    /* "段" only exists in the Small typography, so the unit lives in the
     * title and the big value stays numeric. */
    setupLabel(beamClipsTitle, 30, CamY + 194, 132, 18, 1, "已录 (段)", ColorMuted);
    add(beamClipsTitle);
    setupLabel(beamClipsValue, 30, CamY + 216, 132, 26, 2, "0", ColorText);
    add(beamClipsValue);
    setupLabel(beamHintLabel, 30, CamY + 248, 132, 20, 1, "触摸点选方位", ColorMuted);
    add(beamHintLabel);

    /* Speaker row 1: live monitor switch (MAX98357A on I2S2). */
    beamMonBtn.setPosition(16, CamY + 292, 160, 64);
    beamMonBtn.setStyle(ColorPanel, 12U);
    beamMonBtn.setBorder(ColorLine, true);
    add(beamMonBtn);
    setupLabel(beamMonLabel, 16, CamY + 314, 160, 20, 1, "监听 关", ColorMuted, AppTextLabel::ALIGN_CENTER);
    add(beamMonLabel);
    beamMonTouch.setPosition(16, CamY + 292, 160, 64);
    beamMonTouch.setAction(beamPressedCallback);
    add(beamMonTouch);

    /* Speaker row 2: latest-clip playback. */
    beamPlayBtn.setPosition(16, CamY + 368, 160, 64);
    beamPlayBtn.setStyle(ColorPanel, 12U);
    beamPlayBtn.setBorder(ColorLine, true);
    add(beamPlayBtn);
    setupLabel(beamPlayLabel, 16, CamY + 390, 160, 20, 1, "回放", ColorMuted, AppTextLabel::ALIGN_CENTER);
    add(beamPlayLabel);
    beamPlayTouch.setPosition(16, CamY + 368, 160, 64);
    beamPlayTouch.setAction(beamPressedCallback);
    add(beamPlayTouch);

    /* Right rail: direction card (readout + dial). */
    beamDirCard.setPosition(RailX, 60, RailW, 214);
    beamDirCard.setStyle(ColorPanel, 10U);
    beamDirCard.setBorder(ColorLine, true);
    add(beamDirCard);
    setupLabel(beamDirTitle, RailX + 14, 68, 144, 18, 1, "方位 · 俯仰", ColorMuted);
    add(beamDirTitle);
    setupLabel(beamThetaLabel, RailX + 14, 88, 68, 24, 2, "+0°", ColorText);
    add(beamThetaLabel);
    setupLabel(beamPhiLabel, RailX + 88, 88, 70, 24, 2, "+0°", ColorText);
    add(beamPhiLabel);

    /* Dial: square face + crosshair + steering dot. Face 120x120 centred. */
    {
        const int16_t dialX = static_cast<int16_t>(RailX + ((RailW - 120) / 2));
        const int16_t dialY = 128;

        beamDialFace.setPosition(dialX, dialY, 120, 120);
        beamDialFace.setStyle(ColorPanel2, 16U);
        beamDialFace.setBorder(ColorBlueDim, true);
        add(beamDialFace);
        beamDialCrossH.setPosition(dialX + 6, dialY + 59, 108, 2);
        beamDialCrossH.setColor(ColorLine);
        add(beamDialCrossH);
        beamDialCrossV.setPosition(dialX + 59, dialY + 6, 2, 108);
        beamDialCrossV.setColor(ColorLine);
        add(beamDialCrossV);
        beamDialDot.setPosition(dialX + 56, dialY + 56, 8, 8);
        beamDialDot.setColor(ColorBlue);
        add(beamDialDot);
    }

    /* Right rail: level card. */
    beamLevelCard.setPosition(RailX, 282, RailW, 108);
    beamLevelCard.setStyle(ColorPanel, 10U);
    beamLevelCard.setBorder(ColorLine, true);
    add(beamLevelCard);
    /* Unit in the Small-typography title: the big Title face has no 'd'. */
    setupLabel(beamLevelTitle, RailX + 14, 290, 144, 18, 1, "电平 dBFS", ColorMuted);
    add(beamLevelTitle);
    setupLabel(beamLevelValue, RailX + 14, 312, 144, 30, 3, "--", ColorText);
    add(beamLevelValue);
    beamLevelTrack.setPosition(RailX + 14, 360, 144, 10);
    beamLevelTrack.setColor(ColorPanel2);
    add(beamLevelTrack);
    beamLevelFill.setPosition(RailX + 14, 360, 2, 10);
    beamLevelFill.setColor(ColorGreen);
    add(beamLevelFill);

    /* Right rail: state card. */
    beamStateCard.setPosition(RailX, 398, RailW, 110);
    beamStateCard.setStyle(ColorPanel, 10U);
    beamStateCard.setBorder(ColorLine, true);
    add(beamStateCard);
    setupLabel(beamStateTitle, RailX + 14, 406, 144, 18, 1, "状态", ColorMuted);
    add(beamStateTitle);
    static const char* stateDefaults[3] = {"48k · 32麦", "WAV 录音", "待机"};
    for (uint32_t i = 0U; i < 3U; ++i)
    {
        setupLabel(beamStateLine[i],
                   RailX + 14,
                   static_cast<int16_t>(428 + (i * 24)),
                   144,
                   20,
                   1,
                   stateDefaults[i],
                   ColorText);
        add(beamStateLine[i]);
    }

    /* Bottom strip: rolling beam-level trace. */
    beamHistory.setPosition(CamX, 548, CamW, 46);
    add(beamHistory);
}

void TemplateView::setupMicPage()
{
    setupLabel(micTitle, ContentX + 24, 64, 400, 26, 2, "麦克风阵列", ColorText);
    add(micTitle);

    for (uint32_t i = 0U; i < MicCount; ++i)
    {
        const int16_t col = static_cast<int16_t>(i % 8U);
        const int16_t row = static_cast<int16_t>(i / 8U);
        const int16_t x = static_cast<int16_t>(ContentX + 24 + (col * 112));
        const int16_t y = static_cast<int16_t>(108 + (row * 82));

        micCell[i].setPosition(x, y, 100, 70);
        micCell[i].setStyle(levelColor(0U), 8U);
        micCell[i].setBorder(ColorLine, true);
        add(micCell[i]);

        char idx[8];
        (void)snprintf(idx, sizeof(idx), "M%02lu", static_cast<unsigned long>(i));
        setupLabel(micIndexLabel[i], x, static_cast<int16_t>(y + 10), 100, 20, 1, idx, ColorMuted, AppTextLabel::ALIGN_CENTER);
        add(micIndexLabel[i]);

        setupLabel(micDbLabel[i], x, static_cast<int16_t>(y + 34), 100, 22, 1, "--", ColorText, AppTextLabel::ALIGN_CENTER);
        add(micDbLabel[i]);
    }

    setupLabel(micSummary[0], ContentX + 24, 448, 260, 24, 1, "阵列状态 等待", ColorAmber);
    setupLabel(micSummary[1], ContentX + 300, 448, 200, 24, 1, "有效 --/32", ColorText);
    setupLabel(micSummary[2], ContentX + 24, 480, 260, 24, 1, "平均 -- dBFS", ColorMuted);
    setupLabel(micSummary[3], ContentX + 300, 480, 200, 24, 1, "峰值 -- dBFS", ColorMuted);
    for (uint32_t i = 0U; i < 4U; ++i)
    {
        add(micSummary[i]);
    }
}

void TemplateView::setupSystemPage()
{
    setupLabel(sysTitle, ContentX + 24, 64, 400, 26, 2, "系统状态", ColorText);
    add(sysTitle);

    sysPerfCard.setPosition(ContentX + 24, 104, 520, 440);
    sysPerfCard.setStyle(ColorPanel, 12U);
    sysPerfCard.setBorder(ColorLine, true);
    add(sysPerfCard);

    /* Enclosure silkscreen sonar arcs as a card watermark. */
    sysDecoSonar.setBitmap(touchgfx::Bitmap(BITMAP_DECO_SONAR_ID));
    sysDecoSonar.setPosition(static_cast<int16_t>(ContentX + 144), 336, 280, 190);
    sysDecoSonar.setAlpha(45U);
    add(sysDecoSonar);

    sysInfoCard.setPosition(ContentX + 560, 104, 356, 440);
    sysInfoCard.setStyle(ColorPanel, 12U);
    sysInfoCard.setBorder(ColorLine, true);
    add(sysInfoCard);

    static const char* perfNames[PerfCount] = {"预处理", "FFT", "GCC", "SRP 搜索", "总计"};
    for (uint32_t i = 0U; i < PerfCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(140 + (i * 78));
        setupLabel(perfName[i], ContentX + 48, y, 130, 22, 1, perfNames[i], ColorText);
        add(perfName[i]);

        perfTrack[i].setPosition(ContentX + 180, static_cast<int16_t>(y + 2), 240, 16);
        perfTrack[i].setColor(ColorPanel2);
        add(perfTrack[i]);

        perfFill[i].setPosition(ContentX + 180, static_cast<int16_t>(y + 2), 12, 16);
        perfFill[i].setColor(ColorBlue);
        add(perfFill[i]);

        setupLabel(perfValue[i], ContentX + 434, y, 96, 22, 1, "--", ColorMuted, AppTextLabel::ALIGN_RIGHT);
        add(perfValue[i]);
    }

    static const char* infoNames[SysInfoCount] = {"电池", "系统电压", "电流", "触摸", "相机帧", "显示错误", "热图渲染", "录像编码", "触发次数", "版本"};
    for (uint32_t i = 0U; i < SysInfoCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(136 + (i * 41));
        setupLabel(sysInfoName[i], ContentX + 584, y, 110, 22, 1, infoNames[i], ColorMuted);
        setupLabel(sysInfoValue[i], ContentX + 690, y, 200, 22, 1, "--", ColorText, AppTextLabel::ALIGN_RIGHT);
        add(sysInfoName[i]);
        add(sysInfoValue[i]);
    }

    /* Hardware-acceleration credit line (the N6 story for the judges). */
    setupLabel(sysAccelLabel, ContentX + 24, 560, 892, 22, 1,
               "N6 硬件加速: GPU2D 渲染 · DMA2D · JPEG 编码 · Helium DSP · NPU 就绪",
               ColorBlueDim, AppTextLabel::ALIGN_CENTER);
    add(sysAccelLabel);

    /* Power + USB controls at the bottom of the perf card. */
    sysRebootBtn.setPosition(ContentX + 48, 496, 148, 38);
    sysRebootBtn.setStyle(ColorPanel2, 10U);
    sysRebootBtn.setBorder(ColorLine, true);
    add(sysRebootBtn);
    setupLabel(sysRebootLabel, ContentX + 48, 504, 148, 22, 1, "重启", ColorText, AppTextLabel::ALIGN_CENTER);
    add(sysRebootLabel);
    sysRebootTouch.setPosition(ContentX + 48, 496, 148, 38);
    sysRebootTouch.setAction(systemPressedCallback);
    add(sysRebootTouch);

    sysPowerBtn.setPosition(ContentX + 212, 496, 148, 38);
    sysPowerBtn.setStyle(ColorPanel2, 10U);
    sysPowerBtn.setBorder(ColorRedDim, true);
    add(sysPowerBtn);
    setupLabel(sysPowerLabel, ContentX + 212, 504, 148, 22, 1, "关机", ColorText, AppTextLabel::ALIGN_CENTER);
    add(sysPowerLabel);
    sysPowerTouch.setPosition(ContentX + 212, 496, 148, 38);
    sysPowerTouch.setAction(systemPressedCallback);
    add(sysPowerTouch);

    /* USB mass-storage handover: SD is lent to the PC while active. */
    sysUsbBtn.setPosition(ContentX + 376, 496, 148, 38);
    sysUsbBtn.setStyle(ColorPanel2, 10U);
    sysUsbBtn.setBorder(ColorBlueDim, true);
    add(sysUsbBtn);
    setupLabel(sysUsbLabel, ContentX + 376, 504, 148, 22, 1, "USB 存储", ColorBlue, AppTextLabel::ALIGN_CENTER);
    add(sysUsbLabel);
    sysUsbTouch.setPosition(ContentX + 376, 496, 148, 38);
    sysUsbTouch.setAction(systemPressedCallback);
    add(sysUsbTouch);
}

void TemplateView::setupParamsPage()
{
    setupLabel(paramsTitle, ContentX + 24, 64, 400, 26, 2, "参数", ColorText);
    add(paramsTitle);

    setupLabel(paramsProfileCaption, ContentX + 24, 108, 300, 22, 1, "显示模式", ColorMuted);
    add(paramsProfileCaption);

    static const char* chipNames[ProfileCount] = {"快速", "标准", "质量"};
    for (uint32_t i = 0U; i < ProfileCount; ++i)
    {
        const int16_t x = static_cast<int16_t>(ContentX + 24 + (i * 136));
        profileChip[i].setPosition(x, 136, 120, 44);
        profileChip[i].setStyle(ColorPanel2, 12U);
        profileChip[i].setBorder(ColorLine, true);
        add(profileChip[i]);

        setupLabel(profileChipLabel[i], x, 146, 120, 24, 2, chipNames[i], ColorText, AppTextLabel::ALIGN_CENTER);
        add(profileChipLabel[i]);

        profileTouch[i].setPosition(x, 136, 120, 44);
        profileTouch[i].setAction(profilePressedCallback);
        add(profileTouch[i]);
    }

    /* Two columns: measurement setup (left) and heat rendering (right). */
    const int16_t leftX = static_cast<int16_t>(ContentX + 24);
    const int16_t rightX = static_cast<int16_t>(ContentX + 478);
    const int16_t colW = 430;

    static const char* rowNames[ParamRowCount] = {"场景模式", "频带", "温度 / 声速", "调色板", "声源轨迹"};
    static const char* rowValues[ParamRowCount] = {"通用", "563 - 7875 Hz", "25℃ / 346 m/s", "铁红", "关闭"};
    for (uint32_t i = 0U; i < ParamRowCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(200 + (i * 72));
        paramRowPanel[i].setPosition(leftX, y, colW, 60);
        paramRowPanel[i].setStyle(ColorPanel, 12U);
        paramRowPanel[i].setBorder(ColorLine, true);
        add(paramRowPanel[i]);

        setupLabel(paramRowName[i], static_cast<int16_t>(leftX + 18), static_cast<int16_t>(y + 20), 130, 22, 1, rowNames[i], ColorMuted);
        /* Temperature row keeps space free on the right for the steppers. */
        const int16_t valueWidth = (i == 2U) ? 148 : 258;
        setupLabel(paramRowValue[i], static_cast<int16_t>(leftX + 152), static_cast<int16_t>(y + 18), valueWidth, 24, 2, rowValues[i], ColorText, AppTextLabel::ALIGN_RIGHT);
        add(paramRowName[i]);
        add(paramRowValue[i]);

        /* Scene, band-mode, palette and trail rows toggle on tap anywhere
         * in the row. */
        if ((i == 0U) || (i == 1U) || (i == 3U) || (i == 4U))
        {
            paramRowTouch[i].setPosition(leftX, y, colW, 60);
            paramRowTouch[i].setAction(paramsPressedCallback);
            add(paramRowTouch[i]);
        }
    }

    static const char* stepText[2] = {"-", "+"};

    /* Temperature steppers on row 2. */
    for (uint32_t i = 0U; i < 2U; ++i)
    {
        const int16_t x = static_cast<int16_t>(leftX + colW - 104 + (i * 52));
        const int16_t y = static_cast<int16_t>(200 + (2U * 72) + 8);
        tempStepChip[i].setPosition(x, y, 44, 44);
        tempStepChip[i].setStyle(ColorPanel2, 12U);
        tempStepChip[i].setBorder(ColorBlueDim, true);
        add(tempStepChip[i]);

        setupLabel(tempStepLabel[i], x, static_cast<int16_t>(y + 9), 44, 26, 2, stepText[i], ColorBlue, AppTextLabel::ALIGN_CENTER);
        add(tempStepLabel[i]);

        tempStepTouch[i].setPosition(x, y, 44, 44);
        tempStepTouch[i].setAction(paramsPressedCallback);
        add(tempStepTouch[i]);
    }

    /* Heat rendering parameter steppers (right column). */
    setupLabel(paramsRenderCaption, rightX, 172, 200, 22, 1, "热图渲染", ColorMuted);
    add(paramsRenderCaption);

    static const char* renderNames[RenderRowCount] = {"dB 下限", "伽马", "噪声门", "平滑"};
    for (uint32_t i = 0U; i < RenderRowCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(200 + (i * 72));
        renderRowPanel[i].setPosition(rightX, y, colW, 60);
        renderRowPanel[i].setStyle(ColorPanel, 12U);
        renderRowPanel[i].setBorder(ColorLine, true);
        add(renderRowPanel[i]);

        setupLabel(renderRowName[i], static_cast<int16_t>(rightX + 18), static_cast<int16_t>(y + 20), 110, 22, 1, renderNames[i], ColorMuted);
        setupLabel(renderRowValue[i], static_cast<int16_t>(rightX + 130), static_cast<int16_t>(y + 18), 176, 24, 2, "--", ColorText, AppTextLabel::ALIGN_RIGHT);
        add(renderRowName[i]);
        add(renderRowValue[i]);

        for (uint32_t j = 0U; j < 2U; ++j)
        {
            const uint32_t idx = (i * 2U) + j;
            const int16_t x = static_cast<int16_t>(rightX + colW - 104 + (j * 52));

            renderStepChip[idx].setPosition(x, static_cast<int16_t>(y + 8), 44, 44);
            renderStepChip[idx].setStyle(ColorPanel2, 12U);
            renderStepChip[idx].setBorder(ColorBlueDim, true);
            add(renderStepChip[idx]);

            setupLabel(renderStepLabel[idx], x, static_cast<int16_t>(y + 17), 44, 26, 2, stepText[j], ColorBlue, AppTextLabel::ALIGN_CENTER);
            add(renderStepLabel[idx]);

            renderStepTouch[idx].setPosition(x, static_cast<int16_t>(y + 8), 44, 44);
            renderStepTouch[idx].setAction(paramsPressedCallback);
            add(renderStepTouch[idx]);
        }
    }

    /* Array mode row: right column, aligned with the left column's last
     * row (tap anywhere in the row to switch Wide32 <-> Core16). */
    {
        const int16_t y = static_cast<int16_t>(200 + (4U * 72));
        arrayModePanel.setPosition(rightX, y, colW, 60);
        arrayModePanel.setStyle(ColorPanel, 12U);
        arrayModePanel.setBorder(ColorLine, true);
        add(arrayModePanel);

        setupLabel(arrayModeName, static_cast<int16_t>(rightX + 18), static_cast<int16_t>(y + 20), 130, 22, 1, "阵列模式", ColorMuted);
        setupLabel(arrayModeValue, static_cast<int16_t>(rightX + 152), static_cast<int16_t>(y + 18), 258, 24, 2, "Wide32 · 48k", ColorText, AppTextLabel::ALIGN_RIGHT);
        add(arrayModeName);
        add(arrayModeValue);

        arrayModeTouch.setPosition(rightX, y, colW, 60);
        arrayModeTouch.setAction(paramsPressedCallback);
        add(arrayModeTouch);
    }
}

void TemplateView::setupMediaPage()
{
    setupLabel(mediaTitle, ContentX + 24, 64, 220, 26, 2, "媒体中心", ColorText);
    add(mediaTitle);

    setupLabel(mediaCountLabel, ContentX + 400, 68, 516, 22, 1, "", ColorMuted, AppTextLabel::ALIGN_RIGHT);
    add(mediaCountLabel);

    /* 4x2 thumbnail grid, tiles at the sidecar's native 16:9 shape. */
    for (uint32_t i = 0U; i < MediaThumbSlots; ++i)
    {
        const int16_t x = static_cast<int16_t>(ContentX + 24 + ((i % 4U) * 226));
        const int16_t y = static_cast<int16_t>(104 + ((i / 4U) * 156));

        mediaThumb[i].setPosition(x, y, 210, 120);
        mediaThumb[i].setColors(rgb(10, 16, 24), ColorLine);
        add(mediaThumb[i]);

        mediaThumbBadge[i].setPosition(static_cast<int16_t>(x + 4), static_cast<int16_t>(y + 4), 44, 18);
        mediaThumbBadge[i].setColor(ColorRed);
        mediaThumbBadge[i].setVisible(false);
        add(mediaThumbBadge[i]);

        setupLabel(mediaThumbLabel[i], x, static_cast<int16_t>(y + 124), 210, 20, 1, "", ColorMuted, AppTextLabel::ALIGN_CENTER);
        add(mediaThumbLabel[i]);

        mediaThumbTouch[i].setPosition(x, y, 210, 144);
        mediaThumbTouch[i].setAction(mediaPressedCallback);
        add(mediaThumbTouch[i]);
    }

    setupLabel(mediaEmptyLabel, ContentX + 24, 240, 892, 26, 2, "暂无媒体文件, 请先截屏或录像", ColorMuted, AppTextLabel::ALIGN_CENTER);
    mediaEmptyLabel.setVisible(false);
    add(mediaEmptyLabel);

    /* page navigation */
    static const char* pageLabels[2] = {"上一页", "下一页"};
    for (uint32_t i = 0U; i < 2U; ++i)
    {
        const int16_t x = static_cast<int16_t>(ContentX + 24 + (i * 780));
        mediaPageBtn[i].setPosition(x, 420, 112, 40);
        mediaPageBtn[i].setStyle(ColorPanel2, 12U);
        mediaPageBtn[i].setBorder(ColorLine, true);
        add(mediaPageBtn[i]);

        setupLabel(mediaPageBtnLabel[i], x, 430, 112, 22, 1, pageLabels[i], ColorText, AppTextLabel::ALIGN_CENTER);
        add(mediaPageBtnLabel[i]);

        mediaPageTouch[i].setPosition(x, 420, 112, 40);
        mediaPageTouch[i].setAction(mediaPressedCallback);
        add(mediaPageTouch[i]);
    }

    static const char* actions[MediaActionCount] = {"截屏", "录像", "播放", "查看", "同步"};
    static const uint16_t icons[MediaActionCount] = {
        BITMAP_UI_SNAPSHOT_ID,
        BITMAP_UI_RECORD_ID,
        BITMAP_UI_MEDIA_ID,
        BITMAP_UI_SD_ID,
        BITMAP_UI_SETTINGS_ID
    };
    for (uint32_t i = 0U; i < MediaActionCount; ++i)
    {
        const int16_t x = static_cast<int16_t>(ContentX + 24 + (i * 150));
        mediaButton[i].setPosition(x, 480, 134, 52);
        mediaButton[i].setStyle(ColorPanel2, 12U);
        mediaButton[i].setBorder(ColorLine, true);
        add(mediaButton[i]);

        mediaButtonIcon[i].setBitmap(touchgfx::Bitmap(icons[i]));
        mediaButtonIcon[i].setPosition(static_cast<int16_t>(x + 18), 494, 24, 24);
        add(mediaButtonIcon[i]);

        setupLabel(mediaButtonLabel[i], static_cast<int16_t>(x + 48), 495, 78, 22, 1, actions[i], ColorText);
        add(mediaButtonLabel[i]);

        mediaTouch[i].setPosition(x, 480, 134, 52);
        mediaTouch[i].setAction(mediaPressedCallback);
        add(mediaTouch[i]);
    }

    /* fullscreen viewer overlay (scrim + scaled preview + caption) */
    viewerScrim.setPosition(0, 0, ScreenW, ScreenH);
    viewerScrim.setColor(rgb(2, 4, 8));
    viewerScrim.setVisible(false);
    add(viewerScrim);

    mediaPreview.setPosition(62, 70, 900, 466);
    mediaPreview.setColors(rgb(10, 16, 24), ColorLine);
    mediaPreview.setVisible(false);
    add(mediaPreview);

    setupLabel(viewerCaption, 62, 548, 900, 22, 1, "", ColorMuted, AppTextLabel::ALIGN_CENTER);
    viewerCaption.setVisible(false);
    add(viewerCaption);

    viewerTouch.setPosition(0, 0, ScreenW, ScreenH);
    viewerTouch.setAction(mediaPressedCallback);
    viewerTouch.setVisible(false);
    add(viewerTouch);
}

void TemplateView::setupBootPage()
{
    /* Layout lives in the top-left 800x480 so the temporary 4.3" panel
     * (which scans out only that region) shows the whole sequence; on the
     * full 1024x600 panel it reads as a slightly left-weighted composition
     * balanced by the wave artwork on the right. */
    constexpr int16_t BootCX = 400; /* column centre */

    bootBg.setPosition(0, 0, ScreenW, ScreenH);
    bootBg.setColor(ColorBg);
    add(bootBg);

    /* Enclosure silkscreen wave as a subtle backdrop on the free right side. */
    bootDecoWave.setBitmap(touchgfx::Bitmap(BITMAP_DECO_WAVE_ID));
    bootDecoWave.setPosition(700, 300, 900, 186);
    bootDecoWave.setAlpha(70U);
    add(bootDecoWave);

    /* Competition badge, top-centre above the emblem. */
    bootCompBadge.setBitmap(touchgfx::Bitmap(BITMAP_DECO_COMP_ID));
    bootCompBadge.setPosition(static_cast<int16_t>(BootCX - 280), 10, 560, 89);
    bootCompBadge.setAlpha(210U);
    add(bootCompBadge);

    /* Static rings (alpha fixed): every animated redraw of the large ring
     * bitmaps raced the single-buffer scanout and flickered; motion comes
     * from the small orbit dot instead. */
    static const int16_t ringDiameter[BootRingCount] = {150, 206, 262};
    static const uint8_t ringAlpha[BootRingCount] = {96U, 56U, 30U};
    for (uint32_t i = 0U; i < BootRingCount; ++i)
    {
        const int16_t d = ringDiameter[i];
        bootRing[i].setBitmap(touchgfx::Bitmap(BITMAP_BOOT_RING_ID));
        bootRing[i].setScalingAlgorithm(touchgfx::ScalableImage::BILINEAR_INTERPOLATION);
        bootRing[i].setPosition(static_cast<int16_t>(BootCX - (d / 2)),
                                static_cast<int16_t>(188 - (d / 2)),
                                d,
                                d);
        bootRing[i].setAlpha(ringAlpha[i]);
        add(bootRing[i]);
    }

    bootEmblem.setBitmap(touchgfx::Bitmap(BITMAP_BOOT_EMBLEM_ID));
    bootEmblem.setPosition(static_cast<int16_t>(BootCX - 72), 122, 144, 132);
    bootEmblem.setAlpha(0U);
    add(bootEmblem);

    bootOrbitDot.setPosition(static_cast<int16_t>(BootCX - 4), 81, 8, 8);
    bootOrbitDot.setColor(ColorBlue);
    add(bootOrbitDot);

    setupLabel(bootTitle, static_cast<int16_t>(BootCX - 200), 290, 400, 40, 3, "声学成像仪", ColorText, AppTextLabel::ALIGN_CENTER);
    add(bootTitle);

    setupLabel(bootSubtitle, static_cast<int16_t>(BootCX - 200), 334, 400, 20, 1, "ACOUSTIC CAMERA · STM32N6", ColorMuted, AppTextLabel::ALIGN_CENTER);
    add(bootSubtitle);

    static const char* itemNames[BootItemCount] = {"电源管理", "相机", "麦克风阵列", "声学引擎", "媒体存储"};
    for (uint32_t i = 0U; i < BootItemCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(360 + (i * 20));
        bootItemDot[i].setPosition(static_cast<int16_t>(BootCX - 130), static_cast<int16_t>(y + 6), 8, 8);
        bootItemDot[i].setColor(ColorMuted);
        add(bootItemDot[i]);

        setupLabel(bootItemName[i], static_cast<int16_t>(BootCX - 110), y, 130, 20, 1, itemNames[i], ColorMuted);
        setupLabel(bootItemState[i], static_cast<int16_t>(BootCX + 8), y, 122, 20, 1, "等待", ColorMuted, AppTextLabel::ALIGN_RIGHT);
        add(bootItemName[i]);
        add(bootItemState[i]);
    }

    bootBarTrack.setPosition(static_cast<int16_t>(BootCX - 130), 468, 260, 4);
    bootBarTrack.setColor(ColorPanel2);
    add(bootBarTrack);

    bootBarFill.setPosition(static_cast<int16_t>(BootCX - 130), 468, 2, 4);
    bootBarFill.setColor(ColorBlue);
    add(bootBarFill);

    /* Version sits top-right where the 4.3" panel still shows it. */
    setupLabel(bootVersion, 560, 14, 230, 20, 1, "N647 · FW " __DATE__, ColorMuted, AppTextLabel::ALIGN_RIGHT);
    add(bootVersion);
}

/* ------------------------------------------------------------------ */
/* animation                                                           */
/* ------------------------------------------------------------------ */

void TemplateView::invalidateRingBand(int16_t x, int16_t y, int16_t diameter)
{
    /* The ring artwork is a thin circle: only the outline strips need a
     * redraw. Strip thickness covers the ring stroke plus its soft glow. */
    const int16_t band = static_cast<int16_t>(10 + (diameter / 8));
    touchgfx::Rect top(x, y, diameter, band);
    touchgfx::Rect bottom(x, static_cast<int16_t>(y + diameter - band), diameter, band);
    touchgfx::Rect left(x, static_cast<int16_t>(y + band), band, static_cast<int16_t>(diameter - (2 * band)));
    touchgfx::Rect right(static_cast<int16_t>(x + diameter - band), static_cast<int16_t>(y + band), band, static_cast<int16_t>(diameter - (2 * band)));

    touchgfx::Application::getInstance()->invalidateArea(top);
    touchgfx::Application::getInstance()->invalidateArea(bottom);
    touchgfx::Application::getInstance()->invalidateArea(left);
    touchgfx::Application::getInstance()->invalidateArea(right);
}

/* GDB remote UI actions (blind-debug hook): write a code, the View performs
 * the same handler a touch would. 1=menu 2=shot 3=rec 4=trigger 5=palette
 * 6=profile-cycle 7=viewer-open(slot0) 8=viewer-close 9=reboot 10=poweroff. */
extern "C" {
volatile uint32_t g_app_ui_test_action = 0U;
}

void TemplateView::handleTickEvent()
{
    if (powerConfirmTicks != 0U)
    {
        --powerConfirmTicks;
        if (powerConfirmTicks == 0U)
        {
            sysPowerLabel.setText("关机");
            sysPowerLabel.setColors(ColorText, ColorBg, false);
        }
    }

    if (g_app_ui_test_action != 0U)
    {
        const uint32_t action = g_app_ui_test_action;
        g_app_ui_test_action = 0U;
        switch (action)
        {
        case 1U: setMenuOpen(!menuOpen); break;
        case 2U: presenter->requestScreenshot(); break;
        case 3U: presenter->toggleRecording(); break;
        case 4U: presenter->toggleTrigger(); break;
        case 5U: presenter->cycleHeatPalette(); break;
        case 6U: presenter->selectProfile(static_cast<uint8_t>((activeProfile + 1U) % ProfileCount)); break;
        case 7U: presenter->selectMediaSlot(0U); setViewerOpen(true); break;
        case 8U: setViewerOpen(false); break;
        case 9U: presenter->rebootSystem(); break;
        case 10U: presenter->powerOffSystem(); break;
        case 11U: presenter->playToggleMedia(); break;
        /* 12+slot: select gallery slot N (remote media E2E testing) */
        case 12U: case 13U: case 14U: case 15U:
            presenter->selectMediaSlot(static_cast<uint8_t>(action - 12U));
            setViewerOpen(true);
            break;
        /* directional recording (remote testing) */
        case 16U: presenter->selectScreen(APP_UI_SCREEN_BEAM); break;
        case 17U: presenter->toggleBeamRecording(); break;
        case 18U: presenter->beamAutoTrack(); break;
        default: break;
        }
    }

    if (activeScreen != APP_UI_SCREEN_BOOT)
    {
        return;
    }

    /* The only boot animation: a small dot orbiting the emblem. Two 20x20
     * invalidations per update - cannot race the scanout the way the old
     * full-ring alpha animation did. */
    constexpr uint16_t Period = 180U;
    bootPhase = static_cast<uint16_t>((bootPhase + 1U) % Period);
    if ((bootPhase % 2U) == 0U)
    {
        const float angle = (6.2831853f * static_cast<float>(bootPhase)) / static_cast<float>(Period);
        const int16_t cx = 400;
        const int16_t cy = 188;
        const float r = 107.0f;
        const int16_t nx = static_cast<int16_t>(cx + (r * sinf(angle))) - 4;
        const int16_t ny = static_cast<int16_t>(cy - (r * cosf(angle))) - 4;
        touchgfx::Rect old(bootOrbitDot.getX() - 6, bootOrbitDot.getY() - 6, 20, 20);
        bootOrbitDot.setPosition(nx, ny, 8, 8);
        touchgfx::Rect fresh(nx - 6, ny - 6, 20, 20);
        touchgfx::Application::getInstance()->invalidateArea(old);
        touchgfx::Application::getInstance()->invalidateArea(fresh);
    }

    /* Emblem fade-in in 5 coarse steps instead of 42 fine ones: every
     * emblem invalidate redraws a 144x132 region against the live scanout
     * (single framebuffer), and the per-tick fade was the last remaining
     * boot flicker source. Five spaced redraws still read as a fade. */
    if ((bootEmblemAlpha < 255U) && ((bootPhase % 8U) == 0U))
    {
        const uint16_t next = static_cast<uint16_t>(bootEmblemAlpha + 51U);
        bootEmblemAlpha = (next >= 255U) ? 255U : static_cast<uint8_t>(next);
        bootEmblem.setAlpha(bootEmblemAlpha);
        bootEmblem.invalidate();
    }

    if (bootBarWidth != bootBarTarget)
    {
        int16_t step = static_cast<int16_t>((bootBarTarget - bootBarWidth) / 4);
        if (step == 0)
        {
            step = (bootBarTarget > bootBarWidth) ? 1 : -1;
        }
        bootBarWidth = static_cast<int16_t>(bootBarWidth + step);
        if (bootBarWidth < 2)
        {
            bootBarWidth = 2;
        }
        bootBarFill.setPosition(270, 468, bootBarWidth, 4);
        bootBarTrack.invalidate();
        bootBarFill.invalidate();
    }
}

/* ------------------------------------------------------------------ */
/* refresh                                                             */
/* ------------------------------------------------------------------ */

void TemplateView::updateSnapshot(const AppUiSnapshot& snapshot)
{
    const uint8_t previousScreen = activeScreen;
    activeScreen = snapshot.activeScreen;
    activeProfile = snapshot.activeProfile;

    if (previousScreen != activeScreen)
    {
        refreshVisibility();
        refreshNavigation();
    }

    if (activeScreen == APP_UI_SCREEN_BOOT)
    {
        refreshBootPage(snapshot);
        return;
    }

    refreshStatusBar(snapshot);

    switch (activeScreen)
    {
    case APP_UI_SCREEN_IMAGE:
        refreshImagePage(snapshot);
        break;
    case APP_UI_SCREEN_BEAM:
        refreshBeamPage(snapshot);
        break;
    case APP_UI_SCREEN_MICS:
        refreshMicPage(snapshot);
        break;
    case APP_UI_SCREEN_PERF:
        refreshSystemPage(snapshot);
        break;
    case APP_UI_SCREEN_SETTINGS:
        refreshParamsPage(snapshot);
        break;
    default:
        refreshMediaPage(snapshot);
        break;
    }
}

void TemplateView::refreshVisibility()
{
    const bool bootVisible = (activeScreen == APP_UI_SCREEN_BOOT);
    const bool imageVisible = (activeScreen == APP_UI_SCREEN_IMAGE) && !bootVisible;
    const bool beamVisible = (activeScreen == APP_UI_SCREEN_BEAM);
    const bool micVisible = (activeScreen == APP_UI_SCREEN_MICS);
    const bool sysVisible = (activeScreen == APP_UI_SCREEN_PERF);
    const bool paramsVisible = (activeScreen == APP_UI_SCREEN_SETTINGS);
    const bool mediaVisible = (activeScreen == APP_UI_SCREEN_MEDIA);
    const bool chromeVisible = !bootVisible;

    /* boot */
    bootBg.setVisible(bootVisible);
    bootDecoWave.setVisible(bootVisible);
    bootCompBadge.setVisible(bootVisible);
    for (uint32_t i = 0U; i < BootRingCount; ++i)
    {
        bootRing[i].setVisible(bootVisible);
    }
    bootEmblem.setVisible(bootVisible);
    bootTitle.setVisible(bootVisible);
    bootSubtitle.setVisible(bootVisible);
    for (uint32_t i = 0U; i < BootItemCount; ++i)
    {
        bootItemDot[i].setVisible(bootVisible);
        bootItemName[i].setVisible(bootVisible);
        bootItemState[i].setVisible(bootVisible);
    }
    bootBarTrack.setVisible(bootVisible);
    bootBarFill.setVisible(bootVisible);
    bootOrbitDot.setVisible(bootVisible);
    bootVersion.setVisible(bootVisible);

    /* chrome */
    topBar.setVisible(chromeVisible);
    topBarLine.setVisible(chromeVisible);
    brandMark.setVisible(chromeVisible);
    brandTitle.setVisible(chromeVisible);
    modeChip.setVisible(chromeVisible);
    modeChipLabel.setVisible(chromeVisible);
    recDot.setVisible(chromeVisible && recActive);
    recLabel.setVisible(chromeVisible && recActive);
    beamDot.setVisible(chromeVisible && beamUiRecording);
    sdLabel.setVisible(chromeVisible);
    battLabel.setVisible(chromeVisible);
    fpsLabel.setVisible(chromeVisible);
    for (uint32_t i = 0U; i < 3U; ++i)
    {
        menuBurger[i].setVisible(chromeVisible);
    }
    menuTouch.setVisible(chromeVisible);

    /* popup menu: page switches and boot both close it */
    const bool popupVisible = chromeVisible && menuOpen;
    menuScrim.setVisible(popupVisible);
    menuScrimTouch.setVisible(popupVisible);
    menuPanel.setVisible(popupVisible);
    for (uint32_t i = 0U; i < NavCount; ++i)
    {
        navIcon[i].setVisible(popupVisible);
        navLabel[i].setVisible(popupVisible);
        navTouch[i].setVisible(popupVisible);
        navActiveDot[i].setVisible(popupVisible);
    }

    /* imaging (the color-key hole also serves the beam page's camera view) */
    cameraPreviewKey.setVisible(imageVisible || beamVisible);
    for (uint32_t i = 0U; i < 4U; ++i)
    {
        cameraFrame[i].setVisible(imageVisible);
    }
    for (uint32_t i = 0U; i < 8U; ++i)
    {
        cameraCorner[i].setVisible(imageVisible);
    }
    for (uint32_t i = 0U; i < QuickCount; ++i)
    {
        quickButton[i].setVisible(imageVisible);
        quickIcon[i].setVisible(imageVisible);
        quickLabel[i].setVisible(imageVisible);
        quickTouch[i].setVisible(imageVisible);
    }
    for (uint32_t i = 0U; i < RailCardCount; ++i)
    {
        railCard[i].setVisible(imageVisible);
    }
    railSourceTitle.setVisible(imageVisible);
    railTheta.setVisible(imageVisible);
    railPhi.setVisible(imageVisible);
    railQualityTrack.setVisible(imageVisible);
    railQualityFill.setVisible(imageVisible);
    railStateTitle.setVisible(imageVisible);
    for (uint32_t i = 0U; i < RailStateRows; ++i)
    {
        railStateName[i].setVisible(imageVisible);
        railStateValue[i].setVisible(imageVisible);
    }
    railPerfTitle.setVisible(imageVisible);
    railPerfMs.setVisible(imageVisible);
    railPerfFps.setVisible(imageVisible);
    railPerfCam.setVisible(imageVisible);
    railModeTitle.setVisible(imageVisible);
    railModeValue.setVisible(imageVisible);
    railSceneValue.setVisible(imageVisible);
    railCandLabel[0].setVisible(imageVisible);
    railCandLabel[1].setVisible(imageVisible);
    spectrumPanel.setVisible(imageVisible);
    spectrumBandLabel.setVisible(imageVisible);
    aiCard.setVisible(imageVisible);
    aiTitle.setVisible(imageVisible);
    aiClassLabel.setVisible(imageVisible);
    aiConfTrack.setVisible(imageVisible);
    aiConfFill.setVisible(imageVisible);
    aiConfLabel.setVisible(imageVisible);

    /* beam page (directional recording station) */
    for (uint32_t i = 0U; i < 4U; ++i)
    {
        beamCamFrame[i].setVisible(beamVisible);
    }
    beamAimSurface.setVisible(beamVisible);
    beamAimSurface.setTouchable(beamVisible);
    beamRecBtn.setVisible(beamVisible);
    beamRecLabel.setVisible(beamVisible);
    beamRecTouch.setVisible(beamVisible);
    beamAutoBtn.setVisible(beamVisible);
    beamAutoLabel.setVisible(beamVisible);
    beamAutoTouch.setVisible(beamVisible);
    beamClipsCard.setVisible(beamVisible);
    beamClipsTitle.setVisible(beamVisible);
    beamClipsValue.setVisible(beamVisible);
    beamHintLabel.setVisible(beamVisible);
    beamDirCard.setVisible(beamVisible);
    beamDirTitle.setVisible(beamVisible);
    beamThetaLabel.setVisible(beamVisible);
    beamPhiLabel.setVisible(beamVisible);
    beamDialFace.setVisible(beamVisible);
    beamDialCrossH.setVisible(beamVisible);
    beamDialCrossV.setVisible(beamVisible);
    beamDialDot.setVisible(beamVisible);
    beamLevelCard.setVisible(beamVisible);
    beamLevelTitle.setVisible(beamVisible);
    beamLevelValue.setVisible(beamVisible);
    beamLevelTrack.setVisible(beamVisible);
    beamLevelFill.setVisible(beamVisible);
    beamStateCard.setVisible(beamVisible);
    beamStateTitle.setVisible(beamVisible);
    for (uint32_t i = 0U; i < 3U; ++i)
    {
        beamStateLine[i].setVisible(beamVisible);
    }
    beamHistory.setVisible(beamVisible);

    /* array */
    micTitle.setVisible(micVisible);
    for (uint32_t i = 0U; i < MicCount; ++i)
    {
        micCell[i].setVisible(micVisible);
        micIndexLabel[i].setVisible(micVisible);
        micDbLabel[i].setVisible(micVisible);
    }
    for (uint32_t i = 0U; i < 4U; ++i)
    {
        micSummary[i].setVisible(micVisible);
    }

    /* system */
    sysTitle.setVisible(sysVisible);
    sysPerfCard.setVisible(sysVisible);
    sysDecoSonar.setVisible(sysVisible);
    sysAccelLabel.setVisible(sysVisible);
    sysInfoCard.setVisible(sysVisible);
    for (uint32_t i = 0U; i < PerfCount; ++i)
    {
        perfName[i].setVisible(sysVisible);
        perfTrack[i].setVisible(sysVisible);
        perfFill[i].setVisible(sysVisible);
        perfValue[i].setVisible(sysVisible);
    }
    for (uint32_t i = 0U; i < SysInfoCount; ++i)
    {
        sysInfoName[i].setVisible(sysVisible);
        sysInfoValue[i].setVisible(sysVisible);
    }
    sysRebootBtn.setVisible(sysVisible);
    sysRebootLabel.setVisible(sysVisible);
    sysRebootTouch.setVisible(sysVisible);
    sysPowerBtn.setVisible(sysVisible);
    sysPowerLabel.setVisible(sysVisible);
    sysPowerTouch.setVisible(sysVisible);
    sysUsbBtn.setVisible(sysVisible);
    sysUsbLabel.setVisible(sysVisible);
    sysUsbTouch.setVisible(sysVisible);

    /* params */
    paramsTitle.setVisible(paramsVisible);
    paramsProfileCaption.setVisible(paramsVisible);
    for (uint32_t i = 0U; i < ProfileCount; ++i)
    {
        profileChip[i].setVisible(paramsVisible);
        profileChipLabel[i].setVisible(paramsVisible);
        profileTouch[i].setVisible(paramsVisible);
    }
    for (uint32_t i = 0U; i < ParamRowCount; ++i)
    {
        paramRowPanel[i].setVisible(paramsVisible);
        paramRowName[i].setVisible(paramsVisible);
        paramRowValue[i].setVisible(paramsVisible);
        paramRowTouch[i].setVisible(paramsVisible && ((i == 0U) || (i == 1U) || (i == 3U)));
    }
    for (uint32_t i = 0U; i < 2U; ++i)
    {
        tempStepChip[i].setVisible(paramsVisible);
        tempStepLabel[i].setVisible(paramsVisible);
        tempStepTouch[i].setVisible(paramsVisible);
    }
    paramsRenderCaption.setVisible(paramsVisible);
    arrayModePanel.setVisible(paramsVisible);
    arrayModeName.setVisible(paramsVisible);
    arrayModeValue.setVisible(paramsVisible);
    arrayModeTouch.setVisible(paramsVisible);
    for (uint32_t i = 0U; i < RenderRowCount; ++i)
    {
        renderRowPanel[i].setVisible(paramsVisible);
        renderRowName[i].setVisible(paramsVisible);
        renderRowValue[i].setVisible(paramsVisible);
    }
    for (uint32_t i = 0U; i < (RenderRowCount * 2U); ++i)
    {
        renderStepChip[i].setVisible(paramsVisible);
        renderStepLabel[i].setVisible(paramsVisible);
        renderStepTouch[i].setVisible(paramsVisible);
    }

    /* media */
    if (!mediaVisible && viewerOpen)
    {
        viewerOpen = false;
    }
    mediaTitle.setVisible(mediaVisible && !viewerOpen);
    mediaCountLabel.setVisible(mediaVisible && !viewerOpen);
    mediaEmptyLabel.setVisible(false); /* refreshMediaPage decides */
    for (uint32_t i = 0U; i < MediaThumbSlots; ++i)
    {
        /* refreshMediaPage re-shows used tiles */
        mediaThumb[i].setVisible(false);
        mediaThumbLabel[i].setVisible(false);
        mediaThumbBadge[i].setVisible(false);
        mediaThumbTouch[i].setVisible(false);
    }
    for (uint32_t i = 0U; i < 2U; ++i)
    {
        mediaPageBtn[i].setVisible(mediaVisible && !viewerOpen);
        mediaPageBtnLabel[i].setVisible(mediaVisible && !viewerOpen);
        mediaPageTouch[i].setVisible(mediaVisible && !viewerOpen);
    }
    for (uint32_t i = 0U; i < MediaActionCount; ++i)
    {
        mediaButton[i].setVisible(mediaVisible && !viewerOpen);
        mediaButtonIcon[i].setVisible(mediaVisible && !viewerOpen);
        mediaButtonLabel[i].setVisible(mediaVisible && !viewerOpen);
        mediaTouch[i].setVisible(mediaVisible && !viewerOpen);
    }
    viewerScrim.setVisible(mediaVisible && viewerOpen);
    mediaPreview.setVisible(mediaVisible && viewerOpen);
    viewerCaption.setVisible(mediaVisible && viewerOpen);
    viewerTouch.setVisible(mediaVisible && viewerOpen);

    invalidate();
}

void TemplateView::refreshNavigation()
{
    static const uint8_t navScreens[NavCount] = {
        APP_UI_SCREEN_IMAGE,
        APP_UI_SCREEN_BEAM,
        APP_UI_SCREEN_MICS,
        APP_UI_SCREEN_SETTINGS,
        APP_UI_SCREEN_MEDIA,
        APP_UI_SCREEN_PERF
    };

    for (uint32_t i = 0U; i < NavCount; ++i)
    {
        const bool active = (navScreens[i] == activeScreen);
        navLabel[i].setColors(active ? ColorBlue : ColorText, ColorBg, false);
        navActiveDot[i].setVisible(menuOpen && active);
        navActiveDot[i].invalidate();
    }
}

void TemplateView::refreshStatusBar(const AppUiSnapshot& snapshot)
{
    char text[48];

    (void)snprintf(text, sizeof(text), "%s · %s",
                   sceneName(snapshot.acousticScene),
                   profileName(activeProfile));
    modeChipLabel.setText(text);
    modeChipLabel.setColors((snapshot.sourceDisplayValid != 0U) ? ColorBlue : ColorMuted,
                            ColorBg, false);

    const bool recording = (snapshot.mediaFlags & APP_UI_MEDIA_FLAG_RECORDING) != 0U;
    if (recording != recActive)
    {
        recActive = recording;
        recDot.setVisible(recActive);
        recLabel.setVisible(recActive);
        recDot.invalidate();
        recLabel.invalidate();
    }
    if (recording)
    {
        (void)snprintf(text,
                       sizeof(text),
                       "%02lu:%02lu",
                       static_cast<unsigned long>(snapshot.mediaRecordSeconds / 60U),
                       static_cast<unsigned long>(snapshot.mediaRecordSeconds % 60U));
        recLabel.setText(text);
    }

    /* Beam WAV recording tick: status-bar scope so it stays honest when the
     * user browses other pages while the directional recording runs. */
    {
        const bool beamRec = (snapshot.beamRecording != 0U);

        if (beamDot.isVisible() != beamRec)
        {
            beamDot.setVisible(beamRec);
            beamDot.invalidate();
        }
    }

    if ((snapshot.mediaFlags & APP_UI_MEDIA_FLAG_FS_MOUNTED) != 0U)
    {
        const uint32_t freeMb = (snapshot.mediaFreeMb <= snapshot.mediaTotalMb)
                                ? snapshot.mediaFreeMb
                                : snapshot.mediaTotalMb;
        if (freeMb >= 1024U)
        {
            (void)snprintf(text, sizeof(text), "SD %lu.%luG",
                           static_cast<unsigned long>(freeMb / 1024U),
                           static_cast<unsigned long>(((freeMb % 1024U) * 10U) / 1024U));
        }
        else
        {
            (void)snprintf(text, sizeof(text), "SD %luM", static_cast<unsigned long>(freeMb));
        }
        sdLabel.setColors(ColorMuted, ColorBg, false);
    }
    else
    {
        (void)snprintf(text, sizeof(text), "SD --");
        sdLabel.setColors(ColorAmber, ColorBg, false);
    }
    sdLabel.setText(text);

    if (snapshot.batteryMv != 0U)
    {
        (void)snprintf(text, sizeof(text), "%s %u%%", powerStateName(snapshot.powerState), snapshot.batteryPct);
        battLabel.setVisible(true);
        battLabel.setText(text);
    }
    else if (battLabel.isVisible())
    {
        battLabel.setVisible(false);
        battLabel.invalidate();
    }

    if (snapshot.uiFpsX10 != 0U)
    {
        (void)snprintf(text, sizeof(text), "热图 %u.%u fps",
                       snapshot.uiFpsX10 / 10U,
                       snapshot.uiFpsX10 % 10U);
    }
    else
    {
        (void)snprintf(text, sizeof(text), "热图 --");
    }
    fpsLabel.setText(text);

    /* Health toast. Touch failure outranks the mic banner: without touch the
     * user cannot navigate at all, and the on-screen codes are the only
     * diagnosis channel (a dead touch blocks any touch-driven debug UI).
     * >=5 init attempts = ~5 s of 1 Hz retries, well past normal bring-up. */
    const bool onBoot = (activeScreen == APP_UI_SCREEN_BOOT);
    const bool touchFault = (snapshot.touchReady == 0U) &&
                            (snapshot.touchInitAttempts >= 5U);
    const bool recovering = (snapshot.pcmdFlags & APP_UI_PCMD_FLAG_RECOVERING) != 0U;
    const bool show = (touchFault || recovering) && !onBoot;
    if (show)
    {
        if (touchFault)
        {
            (void)snprintf(text, sizeof(text), "触摸异常 E%u H%u N%u",
                           snapshot.touchLastError,
                           snapshot.touchHalStatus,
                           snapshot.touchInitAttempts);
            alertLabel.setText(text);
        }
        else if (snapshot.pcmdWatchdogRestarts >= 3U)
        {
            alertLabel.setText("麦克风阵列异常, 请检查连接");
        }
        else
        {
            alertLabel.setText("麦克风阵列重连中...");
        }
    }
    if (show != alertBanner.isVisible())
    {
        alertBanner.setVisible(show);
        alertLabel.setVisible(show);
        alertBanner.invalidate();
        alertLabel.invalidate();
    }
}

void TemplateView::refreshBootPage(const AppUiSnapshot& snapshot)
{
    /* Boot checklist rows map to bring-up module bit positions. */
    static const uint8_t moduleBits[BootItemCount] = {4U, 5U, 6U, 8U, 10U};

    uint32_t readyCount = 0U;
    uint32_t watchedCount = 0U;

    for (uint32_t i = 0U; i < BootItemCount; ++i)
    {
        const uint32_t mask = 1UL << moduleBits[i];
        const bool enabled = (snapshot.bringupEnabledMask & mask) != 0U;
        uint8_t state = APP_UI_BOOT_MODULE_PENDING;

        if (!enabled)
        {
            if ((snapshot.bringupSkippedMask & mask) != 0U)
            {
                state = APP_UI_BOOT_MODULE_SKIPPED;
            }
        }
        else
        {
            ++watchedCount;
            if ((snapshot.bringupReadyMask & mask) != 0U)
            {
                state = APP_UI_BOOT_MODULE_READY;
                ++readyCount;
            }
            else if ((snapshot.bringupFailedMask & mask) != 0U)
            {
                state = APP_UI_BOOT_MODULE_FAILED;
                ++readyCount; /* resolved, keeps the bar honest */
            }
            else if ((snapshot.bringupSkippedMask & mask) != 0U)
            {
                state = APP_UI_BOOT_MODULE_SKIPPED;
                ++readyCount;
            }
        }

        touchgfx::colortype dotColor = ColorMuted;
        switch (state)
        {
        case APP_UI_BOOT_MODULE_READY:
            dotColor = ColorGreen;
            bootItemState[i].setColors(ColorGreen, ColorBg, false);
            bootItemState[i].setText("就绪");
            break;
        case APP_UI_BOOT_MODULE_FAILED:
            dotColor = ColorRed;
            bootItemState[i].setColors(ColorRed, ColorBg, false);
            bootItemState[i].setText("失败");
            break;
        case APP_UI_BOOT_MODULE_SKIPPED:
            bootItemState[i].setColors(ColorMuted, ColorBg, false);
            bootItemState[i].setText("跳过");
            break;
        default:
            bootItemState[i].setColors(ColorMuted, ColorBg, false);
            bootItemState[i].setText("等待");
            break;
        }
        /* refreshBootPage runs every model tick: redraw a dot only when its
         * state actually changed, not 60 times a second. */
        if (bootItemDot[i].getColor() != dotColor)
        {
            bootItemDot[i].setColor(dotColor);
            bootItemDot[i].invalidate();
        }
    }

    if (watchedCount == 0U)
    {
        bootBarTarget = 24;
    }
    else
    {
        bootBarTarget = static_cast<int16_t>((260U * readyCount) / watchedCount);
        if (bootBarTarget < 24)
        {
            bootBarTarget = 24;
        }
    }
}

void TemplateView::refreshImagePage(const AppUiSnapshot& snapshot)
{
    char text[64];

    if (snapshot.sourceDisplayValid != 0U)
    {
        (void)snprintf(text, sizeof(text), "方位 %+d°", snapshot.thetaDeg);
        railTheta.setText(text);
        railTheta.setColors(ColorText, ColorBg, false);
        (void)snprintf(text, sizeof(text), "俯仰 %+d°", snapshot.phiDeg);
        railPhi.setText(text);
        railPhi.setColors(ColorText, ColorBg, false);
    }
    else
    {
        railTheta.setText("方位 --");
        railTheta.setColors(ColorMuted, ColorBg, false);
        railPhi.setText("俯仰 --");
        railPhi.setColors(ColorMuted, ColorBg, false);
    }

    /* spectrum panel + band readout in the left column */
    spectrumPanel.setBinHz((snapshot.arrayMode != 0U) ? 375.0f : 187.5f);
    spectrumPanel.setData(snapshot.spectrum,
                          snapshot.acousticBandLoHz,
                          snapshot.acousticBandHiHz,
                          snapshot.spectrumPeakBin);
    (void)snprintf(text, sizeof(text), "%s %u-%u Hz",
                   (snapshot.acousticBandMode != 0U)
                       ? "手动"
                       : ((snapshot.acousticBandAutoActive != 0U) ? "自动·追踪" : "自动"),
                   snapshot.acousticBandLoHz,
                   snapshot.acousticBandHiHz);
    spectrumBandLabel.setColors((snapshot.acousticBandMode != 0U) ? ColorAmber : ColorMuted,
                                ColorBg, false);
    spectrumBandLabel.setText(text);

    /* AI signature card */
    {
        static const char* aiNames[6] = {"监听中", "气体泄漏", "轴承异响", "电弧放电", "机械撞击", "环境声"};
        static const touchgfx::colortype aiColors[6] = {
            ColorMuted, ColorRed, ColorAmber, ColorRed, ColorAmber, ColorBlue
        };
        const uint8_t cls = (snapshot.aiClass < 6U) ? snapshot.aiClass : 0U;

        aiClassLabel.setColors(aiColors[cls], ColorBg, false);
        aiClassLabel.setText(aiNames[cls]);

        if (snapshot.aiConfidencePct != 0U)
        {
            (void)snprintf(text, sizeof(text), "%u%%", snapshot.aiConfidencePct);
        }
        else
        {
            text[0] = '\0';
        }
        aiConfLabel.setText(text);

        int16_t confW = static_cast<int16_t>((100 * snapshot.aiConfidencePct) / 100U);
        if (confW < 2)
        {
            confW = 2;
        }
        if (aiConfFill.getWidth() != confW)
        {
            aiConfFill.setPosition(RailX + 14, 574, confW, 5);
            aiConfFill.setColor((cls == 1U || cls == 3U) ? ColorRed : ColorBlue);
            aiConfTrack.invalidate();
            aiConfFill.invalidate();
        }
    }

    /* Quality is SRP peak prominence, typically 0-20% (valid gate at 3%);
     * stretch 0-20% across the whole bar so it reads as confidence. */
    const uint32_t qScaled = (snapshot.qualityPct >= 20U) ? 100U
                             : (static_cast<uint32_t>(snapshot.qualityPct) * 5U);
    int16_t fillW = static_cast<int16_t>((144U * qScaled) / 100U);
    if (fillW < 4)
    {
        fillW = 4;
    }
    if (railQualityFill.getWidth() != fillW)
    {
        railQualityFill.setPosition(RailX + 14, 142, fillW, 5);
        railQualityTrack.invalidate();
        railQualityFill.invalidate();
    }

    /* Secondary sources (multi-source list). */
    for (uint32_t i = 0U; i < 2U; ++i)
    {
        const uint32_t cand = i + 1U;
        if (cand < snapshot.candCount)
        {
            (void)snprintf(text, sizeof(text), "S%lu %+d° / %+d°  %u%%",
                           static_cast<unsigned long>(cand + 1U),
                           snapshot.candTheta[cand],
                           snapshot.candPhi[cand],
                           (snapshot.candStrength[cand] * 100U) / 255U);
        }
        else
        {
            text[0] = '\0';
        }
        railCandLabel[i].setText(text);
    }

    /* Peak level readout folded into the source card title. */
    (void)snprintf(text, sizeof(text), "声源 · 峰值 %d dB", snapshot.pcmdRawPeakDbfs);
    railSourceTitle.setText(text);

    const bool pcmdOk = (snapshot.pcmdFlags & APP_UI_PCMD_FLAG_FRAME_VALID) != 0U;
    const bool pcmdFault = (snapshot.pcmdFlags & APP_UI_PCMD_FLAG_RAW_FAULT) != 0U;
    railStateValue[0].setColors(pcmdFault ? ColorRed : (pcmdOk ? ColorGreen : ColorAmber), ColorBg, false);
    railStateValue[0].setText(pcmdFault ? "故障" : (pcmdOk ? "正常" : "等待"));

    const bool camOk = (snapshot.cameraSwapCount != 0U);
    railStateValue[1].setColors(camOk ? ColorGreen : ColorAmber, ColorBg, false);
    railStateValue[1].setText(camOk ? "显示中" : "等待");

    const bool sdOk = (snapshot.mediaFlags & APP_UI_MEDIA_FLAG_FS_MOUNTED) != 0U;
    railStateValue[2].setColors(sdOk ? ColorGreen : ColorAmber, ColorBg, false);
    railStateValue[2].setText(sdOk ? "已挂载" : "--");

    if (snapshot.acousticProcessedFrames != 0U)
    {
        (void)snprintf(text, sizeof(text), "SRP %u.%02u ms",
                       snapshot.srpMsX100 / 100U,
                       snapshot.srpMsX100 % 100U);
    }
    else
    {
        (void)snprintf(text, sizeof(text), "SRP --");
    }
    railPerfMs.setText(text);

    if (snapshot.uiFpsX10 != 0U)
    {
        (void)snprintf(text, sizeof(text), "热图 %u.%u fps",
                       snapshot.uiFpsX10 / 10U,
                       snapshot.uiFpsX10 % 10U);
    }
    else
    {
        (void)snprintf(text, sizeof(text), "热图 --");
    }
    railPerfFps.setText(text);

    (void)snprintf(text, sizeof(text), "相机 %lu 帧", static_cast<unsigned long>(snapshot.cameraSwapCount));
    railPerfCam.setText(text);

    /* Mode name + its pair budget makes the three profiles visibly
     * different at a glance (96/160/240 pairs). */
    (void)snprintf(text, sizeof(text), "%s · %u对",
                   profileName(activeProfile), snapshot.acousticPairCount);
    railModeValue.setText(text);
    railModeTitle.setText((snapshot.arrayMode != 0U) ? "Core16 · 192k"
                                                     : "Wide32 · 48k");

    /* quick buttons: record + trigger reflect their armed/active state */
    const bool recording = (snapshot.mediaFlags & APP_UI_MEDIA_FLAG_RECORDING) != 0U;
    quickButton[1].setFillColor(recording ? ColorRedDim : ColorPanel2);
    quickLabel[1].setText(recording ? "停止" : "录像");
    quickLabel[1].setColors(recording ? ColorRed : ColorText, ColorBg, false);

    const bool armed = (snapshot.triggerArmed != 0U);
    quickButton[2].setFillColor(armed ? ColorBlueDim : ColorPanel2);
    quickButton[2].setBorder(armed ? ColorBlue : ColorLine, true);
    quickLabel[2].setColors(armed ? ColorBlue : ColorText, ColorBg, false);
    quickLabel[2].setText(armed ? "警戒中" : "触发");

    quickLabel[3].setText(paletteName(snapshot.heatPalette));

    quickLabel[4].setText(profileName(activeProfile));

    railSceneValue.setText(sceneName(snapshot.acousticScene));
}

/* Directional-recording page: buttons, dial, level meter, history, state. */
void TemplateView::refreshBeamPage(const AppUiSnapshot& snapshot)
{
    const bool recording = (snapshot.beamRecording != 0U);
    const bool manual = (snapshot.beamManual != 0U);
    const bool available = (snapshot.arrayMode == 0U);
    char text[32];

    /* Record button (page主键). "停录" not "停止": the Body typography that
     * backs this scale-2 label has no 止 glyph. */
    if (recording)
    {
        (void)snprintf(text, sizeof(text), "停录 %02u:%02u",
                       snapshot.beamSeconds / 60U, snapshot.beamSeconds % 60U);
        beamRecLabel.setText(text);
        beamRecLabel.setColors(ColorText, ColorBg, false);
        beamRecBtn.setFillColor(ColorRed);
    }
    else
    {
        beamRecLabel.setText(available ? "录制" : "限48k");
        beamRecLabel.setColors(available ? ColorRed : ColorMuted, ColorBg, false);
        beamRecBtn.setFillColor(available ? ColorRedDim : ColorPanel2);
    }
    if (recording != beamUiRecording)
    {
        beamUiRecording = recording;
        beamRecBtn.invalidate();
        beamRecLabel.invalidate();
    }

    /* Auto-track / manual toggle. */
    beamAutoLabel.setText(manual ? "点选" : "自动");
    beamAutoLabel.setColors(manual ? ColorAmber : ColorBlue, ColorBg, false);
    beamAutoBtn.setFillColor(manual ? ColorPanel2 : ColorBlueDim);
    beamAutoBtn.setBorder(manual ? ColorAmber : ColorBlue, true);

    /* Direction readout + dial dot (theta -> x, phi -> y, +/-52 px throw). */
    (void)snprintf(text, sizeof(text), "%+d°", snapshot.beamTheta);
    beamThetaLabel.setText(text);
    (void)snprintf(text, sizeof(text), "%+d°", snapshot.beamPhi);
    beamPhiLabel.setText(text);
    {
        const int16_t dialX = static_cast<int16_t>(RailX + ((RailW - 120) / 2));
        const int16_t dialY = 128;
        int32_t dx = (static_cast<int32_t>(snapshot.beamTheta) * 52) / 60;
        int32_t dy = -(static_cast<int32_t>(snapshot.beamPhi) * 52) / 50;

        if (dx < -52) { dx = -52; }
        if (dx > 52) { dx = 52; }
        if (dy < -52) { dy = -52; }
        if (dy > 52) { dy = 52; }

        const int16_t nx = static_cast<int16_t>(dialX + 60 + dx - 4);
        const int16_t ny = static_cast<int16_t>(dialY + 60 + dy - 4);

        if ((beamDialDot.getX() != nx) || (beamDialDot.getY() != ny))
        {
            beamDialDot.invalidate();
            beamDialDot.setPosition(nx, ny, 8, 8);
            beamDialDot.invalidate();
        }
        beamDialDot.setColor(recording ? ColorRed : (manual ? ColorAmber : ColorBlue));
    }

    /* Level: big value, bar, rolling history strip. While the beam compute
     * is not running (192k mode / page just entered) the snapshot level is
     * meaningless - show a quiet placeholder instead of "0 dBFS" with a
     * full red bar. */
    {
        const bool live = (snapshot.beamActive != 0U);
        int32_t level = snapshot.beamRmsDbfs;
        int16_t fillW = 2;

        if (level < -90) { level = -90; }
        if (level > 0) { level = 0; }

        if (live)
        {
            (void)snprintf(text, sizeof(text), "%ld", static_cast<long>(level));
            beamLevelValue.setText(text);
            fillW = static_cast<int16_t>(((level + 90) * 144) / 90);
            if (fillW < 2)
            {
                fillW = 2;
            }
        }
        else
        {
            beamLevelValue.setText("--");
        }

        if (beamLevelFill.getWidth() != fillW)
        {
            beamLevelFill.setPosition(RailX + 14, 360, fillW, 10);
            beamLevelFill.setColor((level > -15) ? ColorRed :
                                   ((level > -35) ? ColorAmber : ColorGreen));
            beamLevelTrack.invalidate();
            beamLevelFill.invalidate();
        }

        beamHistory.setHot(recording);
        beamHistory.push(live ? static_cast<uint8_t>(level + 90) : 0U);
    }

    /* Clip counter + interaction hint. */
    (void)snprintf(text, sizeof(text), "%lu", static_cast<unsigned long>(snapshot.beamClips));
    beamClipsValue.setText(text);
    beamHintLabel.setText(manual ? "按自动回声源" : "触摸点选方位");

    /* Live speaker monitor switch. */
    {
        const bool monitorOn = (snapshot.beamMonitorOn != 0U);

        beamMonLabel.setText(monitorOn ? "监听 开" : "监听 关");
        beamMonLabel.setColors(monitorOn ? ColorGreen : ColorMuted, ColorBg, false);
        beamMonBtn.setFillColor(monitorOn ? ColorPanel2 : ColorPanel);
        beamMonBtn.setBorder(monitorOn ? ColorGreen : ColorLine, true);
    }

    /* Latest-clip playback: shows position while running, count-gated. */
    {
        const bool playing = (snapshot.audioPlaying != 0U);
        const bool playable = (snapshot.beamClips != 0U) && !recording;

        if (playing)
        {
            (void)snprintf(text, sizeof(text), "停 %02u:%02u",
                           snapshot.audioPlaySeconds / 60U,
                           snapshot.audioPlaySeconds % 60U);
            beamPlayLabel.setText(text);
            beamPlayLabel.setColors(ColorAmber, ColorBg, false);
            beamPlayBtn.setFillColor(ColorPanel2);
            beamPlayBtn.setBorder(ColorAmber, true);
        }
        else
        {
            beamPlayLabel.setText("回放");
            beamPlayLabel.setColors(playable ? ColorText : ColorMuted, ColorBg, false);
            beamPlayBtn.setFillColor(ColorPanel);
            beamPlayBtn.setBorder(playable ? ColorLine : ColorPanel2, true);
        }
    }

    /* State card. */
    beamStateLine[0].setText(available ? "48k · 32麦" : "限48k模式");
    beamStateLine[0].setColors(available ? ColorText : ColorAmber, ColorBg, false);
    if (recording)
    {
        (void)snprintf(text, sizeof(text), "REC %02u:%02u",
                       snapshot.beamSeconds / 60U, snapshot.beamSeconds % 60U);
        beamStateLine[1].setText(text);
        beamStateLine[1].setColors(ColorRed, ColorBg, false);
    }
    else
    {
        beamStateLine[1].setText("WAV 待机");
        beamStateLine[1].setColors(ColorMuted, ColorBg, false);
    }
    beamStateLine[2].setText((snapshot.sourceDisplayValid != 0U) ? "有声源" : "无声源");
    beamStateLine[2].setColors((snapshot.sourceDisplayValid != 0U) ? ColorGreen : ColorMuted,
                               ColorBg, false);
}

void TemplateView::refreshMicPage(const AppUiSnapshot& snapshot)
{
    char text[32];
    uint32_t activeCount = 0U;
    int32_t avgSum = 0;
    int8_t peak = -90;

    for (uint32_t i = 0U; i < MicCount; ++i)
    {
        const uint8_t level = snapshot.micLevel[i];
        const int8_t dbfs = snapshot.micDbfs[i];

        micCell[i].setFillColor(levelColor(level));

        if (dbfs > -85)
        {
            ++activeCount;
        }
        avgSum += dbfs;
        if (dbfs > peak)
        {
            peak = dbfs;
        }

        (void)snprintf(text, sizeof(text), "%d", dbfs);
        micDbLabel[i].setText(text);
    }

    const bool rawValid = (snapshot.pcmdFlags & APP_UI_PCMD_FLAG_RAW_VALID) != 0U;
    const bool fault = (snapshot.pcmdFlags & APP_UI_PCMD_FLAG_RAW_FAULT) != 0U;
    micSummary[0].setColors(fault ? ColorRed : (rawValid ? ColorGreen : ColorAmber), ColorBg, false);
    micSummary[0].setText(fault ? "阵列状态 故障" : (rawValid ? "阵列状态 正常" : "阵列状态 等待"));

    (void)snprintf(text, sizeof(text), "有效 %lu/32", static_cast<unsigned long>(activeCount));
    micSummary[1].setText(text);

    (void)snprintf(text, sizeof(text), "平均 %ld dBFS", static_cast<long>(avgSum / 32));
    micSummary[2].setText(text);

    (void)snprintf(text, sizeof(text), "峰值 %d dBFS", peak);
    micSummary[3].setText(text);
}

void TemplateView::refreshSystemPage(const AppUiSnapshot& snapshot)
{
    char text[48];

    const uint32_t cycles[PerfCount] = {
        snapshot.srpPreprocessCycles,
        snapshot.srpFftCycles,
        snapshot.srpGccCycles,
        snapshot.srpCoarseCycles + snapshot.srpFineCycles,
        snapshot.srpTotalCycles
    };

    for (uint32_t i = 0U; i < PerfCount; ++i)
    {
        const uint8_t load = (i < 5U) ? snapshot.perfLoad[i] : 0U;
        int16_t w = static_cast<int16_t>((240 * ((load > 100U) ? 100U : load)) / 100U);
        if (w < 6)
        {
            w = 6;
        }
        if (perfFill[i].getWidth() != w)
        {
            perfFill[i].setPosition(ContentX + 180, perfFill[i].getY(), w, 16);
            perfTrack[i].invalidate();
            perfFill[i].invalidate();
        }

        formatCyclesM(text, sizeof(text), cycles[i]);
        perfValue[i].setText(text);
    }

    (void)snprintf(text, sizeof(text), "%lu.%02luV %u%%",
                   static_cast<unsigned long>(snapshot.batteryMv / 1000U),
                   static_cast<unsigned long>((snapshot.batteryMv % 1000U) / 10U),
                   snapshot.batteryPct);
    sysInfoValue[0].setText(text);

    (void)snprintf(text, sizeof(text), "%lu.%02luV",
                   static_cast<unsigned long>(snapshot.systemMv / 1000U),
                   static_cast<unsigned long>((snapshot.systemMv % 1000U) / 10U));
    sysInfoValue[1].setText(text);

    (void)snprintf(text, sizeof(text), "%+ld mA", static_cast<long>(snapshot.batteryCurrentMa));
    sysInfoValue[2].setText(text);

    if (snapshot.touchReady != 0U)
    {
        (void)snprintf(text, sizeof(text), "X%u Y%u", snapshot.touchX, snapshot.touchY);
        sysInfoValue[3].setText(text);
    }
    else
    {
        sysInfoValue[3].setText("等待");
    }

    (void)snprintf(text, sizeof(text), "%lu", static_cast<unsigned long>(snapshot.cameraSwapCount));
    sysInfoValue[4].setText(text);

    (void)snprintf(text, sizeof(text), "%lu", static_cast<unsigned long>(snapshot.cameraDisplayErrorCount));
    sysInfoValue[5].setText(text);

    /* Overlay render time: DWT cycles at 600 MHz -> ms x10. */
    {
        const uint32_t msX10 = snapshot.overlayDrawCycles / 60000U;
        (void)snprintf(text, sizeof(text), "%lu.%lu ms",
                       static_cast<unsigned long>(msX10 / 10U),
                       static_cast<unsigned long>(msX10 % 10U));
    }
    sysInfoValue[6].setText(text);

    if (snapshot.mediaEncodeMs != 0U)
    {
        (void)snprintf(text, sizeof(text), "%u ms", snapshot.mediaEncodeMs);
    }
    else
    {
        (void)snprintf(text, sizeof(text), "--");
    }
    sysInfoValue[7].setText(text);

    (void)snprintf(text, sizeof(text), "%lu", static_cast<unsigned long>(snapshot.triggerCount));
    sysInfoValue[8].setText(text);

    sysInfoValue[9].setText(__DATE__);

    /* Live NPU telemetry in the acceleration credit line. */
    if (snapshot.npuActive != 0U)
    {
        (void)snprintf(text, sizeof(text), "NPU 运行中 %luk 次 · %u us/次",
                       static_cast<unsigned long>(snapshot.npuInferences / 1000U),
                       snapshot.npuLatencyUs);
        char line[128];
        (void)snprintf(line, sizeof(line),
                       "N6 硬件加速: GPU2D · DMA2D · JPEG · Helium DSP · %s",
                       (snapshot.npuInferences >= 1000U) ? text : "NPU 运行中");
        sysAccelLabel.setText(line);
    }

    /* USB mass-storage state: amber while the PC owns the SD. */
    {
        const bool usbMode = (snapshot.mediaFlags & APP_UI_MEDIA_FLAG_USB_MODE) != 0U;
        sysUsbLabel.setText(usbMode ? "退出 USB" : "USB 存储");
        sysUsbLabel.setColors(usbMode ? ColorAmber : ColorBlue, ColorBg, false);
        sysUsbBtn.setBorder(usbMode ? ColorAmber : ColorBlueDim, true);
    }
}

void TemplateView::refreshParamsPage(const AppUiSnapshot& snapshot)
{
    char text[48];

    for (uint32_t i = 0U; i < ProfileCount; ++i)
    {
        const bool active = (i == activeProfile);
        profileChip[i].setFillColor(active ? ColorBlueDim : ColorPanel2);
        profileChip[i].setBorder(active ? ColorBlue : ColorLine, true);
        profileChipLabel[i].setColors(active ? ColorText : ColorMuted, ColorBg, false);
        profileChip[i].invalidate();
    }

    paramRowValue[0].setText(sceneName(snapshot.acousticScene));

    /* Band row: mode prefix + live band; tap toggles auto/manual. */
    (void)snprintf(text, sizeof(text), "%s %u-%u Hz",
                   (snapshot.acousticBandMode != 0U)
                       ? "手动"
                       : ((snapshot.acousticBandAutoActive != 0U) ? "自动·追踪" : "自动"),
                   snapshot.acousticBandLoHz,
                   snapshot.acousticBandHiHz);
    paramRowValue[1].setColors((snapshot.acousticBandMode != 0U) ? ColorAmber : ColorBlue,
                               ColorBg, false);
    paramRowValue[1].setText(text);

    (void)snprintf(text, sizeof(text), "%d℃ / %u.%u m/s",
                   snapshot.acousticTempC,
                   snapshot.acousticSpeedX10 / 10U,
                   snapshot.acousticSpeedX10 % 10U);
    paramRowValue[2].setText(text);

    paramRowValue[3].setText(paletteName(snapshot.heatPalette));

    paramRowValue[4].setColors((snapshot.trailEnabled != 0U) ? ColorBlue : ColorText, ColorBg, false);
    paramRowValue[4].setText((snapshot.trailEnabled != 0U) ? "开" : "关闭");

    if (snapshot.arraySwitching != 0U)
    {
        arrayModeValue.setColors(ColorAmber, ColorBg, false);
        arrayModeValue.setText("启动中");
    }
    else
    {
        arrayModeValue.setColors((snapshot.arrayMode != 0U) ? ColorBlue : ColorText, ColorBg, false);
        arrayModeValue.setText((snapshot.arrayMode != 0U) ? "Core16 · 192k" : "Wide32 · 48k");
    }

    /* render parameter steppers */
    (void)snprintf(text, sizeof(text), "%d dB", snapshot.fieldDbFloor);
    renderRowValue[0].setText(text);

    (void)snprintf(text, sizeof(text), "%u.%02u",
                   snapshot.fieldGammaX100 / 100U,
                   snapshot.fieldGammaX100 % 100U);
    renderRowValue[1].setText(text);

    (void)snprintf(text, sizeof(text), "0.%02u", snapshot.fieldNoiseGateX100);
    renderRowValue[2].setText(text);

    (void)snprintf(text, sizeof(text), "%u", snapshot.fieldSmoothPasses);
    renderRowValue[3].setText(text);
}

void TemplateView::refreshMediaPage(const AppUiSnapshot& snapshot)
{
    char text[96];

    const bool playing = (snapshot.mediaFlags & APP_UI_MEDIA_FLAG_PLAYING) != 0U;
    mediaButtonLabel[2].setText(playing ? "暂停" : "播放");
    mediaButtonLabel[2].setColors(playing ? ColorBlue : ColorText, ColorBg, false);

    const bool recording = (snapshot.mediaFlags & APP_UI_MEDIA_FLAG_RECORDING) != 0U;
    mediaButtonLabel[1].setText(recording ? "停止" : "录像");
    mediaButtonLabel[1].setColors(recording ? ColorRed : ColorText, ColorBg, false);

    thumbPage = snapshot.mediaThumbPage;
    thumbPageCount = snapshot.mediaThumbPageCount;
    for (uint32_t i = 0U; i < MediaThumbSlots; ++i)
    {
        thumbTypes[i] = snapshot.mediaThumbType[i];
    }

    /* header: counts + page + fs state */
    const bool mounted = (snapshot.mediaFlags & APP_UI_MEDIA_FLAG_FS_MOUNTED) != 0U;
    if (!mounted)
    {
        (void)snprintf(text, sizeof(text), "SD 未挂载");
    }
    else
    {
        (void)snprintf(text, sizeof(text), "%lu 张 · %lu 段 · 第 %lu/%lu 页 · 剩余 %luMB",
                       static_cast<unsigned long>(snapshot.mediaScreenshots),
                       static_cast<unsigned long>(snapshot.mediaVideos),
                       static_cast<unsigned long>(snapshot.mediaThumbPage + 1U),
                       static_cast<unsigned long>(snapshot.mediaThumbPageCount),
                       static_cast<unsigned long>(snapshot.mediaFreeMb));
    }
    mediaCountLabel.setText(text);

    /* thumbnail grid */
    const bool empty = (snapshot.mediaThumbTotal == 0U);
    if (empty != mediaEmptyLabel.isVisible())
    {
        mediaEmptyLabel.setVisible(empty);
        mediaEmptyLabel.invalidate();
    }

    const bool thumbsChanged = (snapshot.mediaThumbGeneration != mediaThumbGeneration);
    for (uint32_t i = 0U; i < MediaThumbSlots; ++i)
    {
        const bool used = (snapshot.mediaThumbUsed[i] != 0U);
        const bool tileVisible = used && (activeScreen == APP_UI_SCREEN_MEDIA) && !viewerOpen;

        if (thumbsChanged)
        {
            mediaThumb[i].setSource(snapshot.mediaThumbPixels[i],
                                    176U,
                                    99U,
                                    used && (snapshot.mediaThumbValid[i] != 0U));
            if (used)
            {
                (void)snprintf(text, sizeof(text), "%s%05lu",
                               (snapshot.mediaThumbType[i] == 2U) ? "VID" : "SCR",
                               static_cast<unsigned long>(snapshot.mediaThumbIndex[i]));
                mediaThumbLabel[i].setText(text);
            }
            else
            {
                mediaThumbLabel[i].setText("");
            }
        }
        const bool highlight = tileVisible && (i == selectedSlot);
        mediaThumb[i].setColors(rgb(10, 16, 24), highlight ? ColorBlue : ColorLine);

        if (tileVisible != mediaThumb[i].isVisible())
        {
            mediaThumb[i].setVisible(tileVisible);
            mediaThumb[i].invalidate();
            mediaThumbLabel[i].setVisible(tileVisible);
            mediaThumbLabel[i].invalidate();
        }
        mediaThumbTouch[i].setVisible(tileVisible);
        const bool badgeVisible = tileVisible && (snapshot.mediaThumbType[i] == 2U);
        if (badgeVisible != mediaThumbBadge[i].isVisible())
        {
            mediaThumbBadge[i].setVisible(badgeVisible);
            mediaThumbBadge[i].invalidate();
        }
        if (thumbsChanged && tileVisible)
        {
            mediaThumb[i].invalidate();
        }
    }
    if (thumbsChanged)
    {
        mediaThumbGeneration = snapshot.mediaThumbGeneration;
    }

    /* viewer overlay content */
    mediaPreview.setSource(snapshot.mediaPreviewPixels,
                           snapshot.mediaPreviewWidth,
                           snapshot.mediaPreviewHeight,
                           snapshot.mediaPreviewValid != 0U);
    if (snapshot.mediaPreviewGeneration != mediaPreviewGeneration)
    {
        mediaPreviewGeneration = snapshot.mediaPreviewGeneration;
        if (viewerOpen)
        {
            mediaPreview.invalidate();
        }
    }
    if (viewerOpen)
    {
        if (snapshot.mediaPreviewFrameCount > 1U)
        {
            (void)snprintf(text, sizeof(text), "%s · 帧 %lu/%lu · 点按空白处返回",
                           snapshot.mediaSelectedFile,
                           static_cast<unsigned long>(snapshot.mediaPreviewFrameIndex + 1U),
                           static_cast<unsigned long>(snapshot.mediaPreviewFrameCount));
        }
        else if ((snapshot.mediaFlags & APP_UI_MEDIA_FLAG_BUSY) != 0U)
        {
            (void)snprintf(text, sizeof(text), "加载中...");
        }
        else
        {
            (void)snprintf(text, sizeof(text), "%s · 点按空白处返回", snapshot.mediaSelectedFile);
        }
        viewerCaption.setText(text);
    }
}

/* ------------------------------------------------------------------ */
/* input                                                               */
/* ------------------------------------------------------------------ */

void TemplateView::onNavPressed(const touchgfx::AbstractButton& source)
{
    static const uint8_t navScreens[NavCount] = {
        APP_UI_SCREEN_IMAGE,
        APP_UI_SCREEN_BEAM,
        APP_UI_SCREEN_MICS,
        APP_UI_SCREEN_SETTINGS,
        APP_UI_SCREEN_MEDIA,
        APP_UI_SCREEN_PERF
    };

    for (uint32_t i = 0U; i < NavCount; ++i)
    {
        if (&source == &navTouch[i])
        {
            setMenuOpen(false);
            presenter->selectScreen(navScreens[i]);
            if (navScreens[i] == APP_UI_SCREEN_MEDIA)
            {
                /* fresh gallery page on entry */
                presenter->requestThumbPage(0U);
            }
            return;
        }
    }
}

void TemplateView::onSystemPressed(const touchgfx::AbstractButton& source)
{
    if (&source == &sysRebootTouch)
    {
        presenter->rebootSystem();
    }
    else if (&source == &sysUsbTouch)
    {
        presenter->toggleUsbStorage();
    }
    else if (&source == &sysPowerTouch)
    {
        if (powerConfirmTicks != 0U)
        {
            presenter->powerOffSystem();
        }
        else
        {
            /* Arm a ~3 s confirm window so a stray tap cannot kill a demo. */
            powerConfirmTicks = 180U;
            sysPowerLabel.setText("确认关机");
            sysPowerLabel.setColors(ColorRed, ColorBg, false);
        }
    }
}

void TemplateView::onMenuPressed(const touchgfx::AbstractButton& source)
{
    /* Brand-area tap toggles; scrim tap always closes. */
    if (&source == &menuScrimTouch)
    {
        setMenuOpen(false);
    }
    else
    {
        setMenuOpen(!menuOpen);
    }
}

void TemplateView::onBandChanged(uint16_t loHz, uint16_t hiHz)
{
    presenter->setBandHz(loHz, hiHz);
}

void TemplateView::onQuickPressed(const touchgfx::AbstractButton& source)
{
    if (&source == &quickTouch[0])
    {
        presenter->requestScreenshot();
    }
    else if (&source == &quickTouch[1])
    {
        presenter->toggleRecording();
    }
    else if (&source == &quickTouch[2])
    {
        presenter->toggleTrigger();
    }
    else if (&source == &quickTouch[3])
    {
        presenter->cycleHeatPalette();
    }
    else if (&source == &quickTouch[4])
    {
        presenter->selectProfile(static_cast<uint8_t>((activeProfile + 1U) % ProfileCount));
    }
}

void TemplateView::onBeamPressed(const touchgfx::AbstractButton& source)
{
    if (&source == &beamAutoTouch)
    {
        presenter->beamAutoTrack();
    }
    else if (&source == &beamRecTouch)
    {
        presenter->toggleBeamRecording();
    }
    else if (&source == &beamMonTouch)
    {
        presenter->toggleBeamMonitor();
    }
    else if (&source == &beamPlayTouch)
    {
        presenter->toggleAudioPlayback();
    }
}

void TemplateView::onBeamAim(int16_t px, int16_t py)
{
    /* Aim-surface coordinates are already camera-local (640x480). */
    presenter->setBeamManualTargetPx(px, py);
}

void TemplateView::onProfilePressed(const touchgfx::AbstractButton& source)
{
    for (uint32_t i = 0U; i < ProfileCount; ++i)
    {
        if (&source == &profileTouch[i])
        {
            presenter->selectProfile(static_cast<uint8_t>(i));
            return;
        }
    }
}

void TemplateView::onParamsPressed(const touchgfx::AbstractButton& source)
{
    if (&source == &arrayModeTouch)
    {
        presenter->toggleArrayMode();
    }
    else if (&source == &paramRowTouch[0])
    {
        presenter->cycleScene();
    }
    else if (&source == &paramRowTouch[1])
    {
        presenter->toggleBandMode();
    }
    else if (&source == &paramRowTouch[3])
    {
        presenter->cycleHeatPalette();
    }
    else if (&source == &paramRowTouch[4])
    {
        presenter->toggleTrail();
    }
    else if (&source == &tempStepTouch[0])
    {
        presenter->adjustTemperature(-1);
    }
    else if (&source == &tempStepTouch[1])
    {
        presenter->adjustTemperature(1);
    }
    else
    {
        for (uint32_t i = 0U; i < (RenderRowCount * 2U); ++i)
        {
            if (&source == &renderStepTouch[i])
            {
                presenter->adjustFieldParam(static_cast<uint8_t>(i / 2U),
                                            ((i % 2U) == 0U) ? -1 : 1);
                return;
            }
        }
    }
}

void TemplateView::onMediaPressed(const touchgfx::AbstractButton& source)
{
    /* viewer overlay: any tap closes it */
    if (&source == &viewerTouch)
    {
        setViewerOpen(false);
        return;
    }

    for (uint32_t i = 0U; i < MediaThumbSlots; ++i)
    {
        if (&source == &mediaThumbTouch[i])
        {
            selectedSlot = static_cast<uint8_t>(i);
            presenter->selectMediaSlot(selectedSlot);
            if (thumbTypes[i] == 2U)
            {
                /* videos auto-play in the viewer (queued after the select) */
                presenter->playToggleMedia();
            }
            setViewerOpen(true);
            return;
        }
    }

    if (&source == &mediaPageTouch[0])
    {
        if (thumbPage > 0U)
        {
            presenter->requestThumbPage(thumbPage - 1U);
        }
        return;
    }
    if (&source == &mediaPageTouch[1])
    {
        if ((thumbPage + 1U) < thumbPageCount)
        {
            presenter->requestThumbPage(thumbPage + 1U);
        }
        return;
    }

    if (&source == &mediaTouch[0])
    {
        presenter->requestScreenshot();
    }
    else if (&source == &mediaTouch[1])
    {
        presenter->toggleRecording();
    }
    else if (&source == &mediaTouch[2])
    {
        presenter->playToggleMedia();
    }
    else if (&source == &mediaTouch[3])
    {
        /* open the currently selected item in the viewer */
        presenter->readSelectedMedia();
        setViewerOpen(true);
    }
    else if (&source == &mediaTouch[4])
    {
        presenter->refreshMedia();
    }
}

void TemplateView::setViewerOpen(bool open)
{
    if (viewerOpen == open)
    {
        return;
    }
    viewerOpen = open;
    viewerScrim.setVisible(open);
    mediaPreview.setVisible(open);
    viewerCaption.setVisible(open);
    viewerTouch.setVisible(open);
    refreshVisibility();
    invalidate();
}
