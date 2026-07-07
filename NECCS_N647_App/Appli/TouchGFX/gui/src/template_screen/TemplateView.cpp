#include <gui/template_screen/TemplateView.hpp>

#include <images/BitmapDatabase.hpp>
#include <touchgfx/Color.hpp>

#include <stdio.h>

#include "app_camera_display.h"

namespace
{
constexpr int16_t CameraPreviewX = 192;
constexpr int16_t CameraPreviewY = 60;
constexpr int16_t CameraPreviewW = 640;
constexpr int16_t CameraPreviewH = 480;
constexpr int16_t ScreenW = 1024;
constexpr int16_t ScreenH = 600;
constexpr int16_t NavW = 176;
constexpr int16_t RightHudX = 848;
constexpr int16_t RightHudW = 160;

const touchgfx::colortype ColorBg = touchgfx::Color::getColorFromRGB(7, 12, 16);
const touchgfx::colortype ColorPanel = touchgfx::Color::getColorFromRGB(13, 20, 25);
const touchgfx::colortype ColorPanel2 = touchgfx::Color::getColorFromRGB(18, 27, 32);
const touchgfx::colortype ColorLine = touchgfx::Color::getColorFromRGB(48, 83, 88);
const touchgfx::colortype ColorCyan = touchgfx::Color::getColorFromRGB(94, 236, 214);
const touchgfx::colortype ColorGreen = touchgfx::Color::getColorFromRGB(86, 220, 144);
const touchgfx::colortype ColorText = touchgfx::Color::getColorFromRGB(226, 238, 233);
const touchgfx::colortype ColorMuted = touchgfx::Color::getColorFromRGB(130, 154, 156);
const touchgfx::colortype ColorAmber = touchgfx::Color::getColorFromRGB(245, 184, 70);
const touchgfx::colortype ColorRed = touchgfx::Color::getColorFromRGB(235, 88, 72);

touchgfx::colortype rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return touchgfx::Color::getColorFromRGB(r, g, b);
}

const char* screenName(uint8_t screen)
{
    switch (screen)
    {
    case APP_UI_SCREEN_IMAGE:
        return "成像";
    case APP_UI_SCREEN_MICS:
        return "阵列";
    case APP_UI_SCREEN_PERF:
        return "性能";
    case APP_UI_SCREEN_SETTINGS:
        return "设置";
    case APP_UI_SCREEN_MEDIA:
        return "媒体";
    default:
        return "成像";
    }
}

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

const char* profileShortName(uint8_t profile)
{
    switch (profile)
    {
    case APP_UI_PROFILE_FAST:
        return "FAST";
    case APP_UI_PROFILE_QUALITY:
        return "QUAL";
    default:
        return "STD";
    }
}

const char* touchName(uint8_t ic)
{
    switch (ic)
    {
    case 1U:
        return "FT5X06";
    case 2U:
        return "GT9XXX";
    default:
        return "等待";
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
        return "等待";
    }
}

const char* selectedMediaName(uint8_t type)
{
    switch (type)
    {
    case 1U:
        return "BMP";
    case 2U:
        return "AVI";
    default:
        return "无";
    }
}

touchgfx::colortype levelColor(uint8_t level)
{
    if (level > 82U)
    {
        return rgb(225, 91, 63);
    }
    if (level > 58U)
    {
        return rgb(214, 170, 65);
    }
    if (level > 28U)
    {
        return rgb(62, 172, 132);
    }
    return rgb(43, 73, 78);
}

void formatCyclesM(char* buffer, uint32_t bufferSize, const char* label, uint32_t cycles)
{
    if ((buffer == 0) || (bufferSize == 0U))
    {
        return;
    }
    (void)snprintf(buffer,
                   bufferSize,
                   "%s %lu.%luM",
                   label,
                   static_cast<unsigned long>(cycles / 1000000UL),
                   static_cast<unsigned long>((cycles / 100000UL) % 10UL));
}

void setupLabel(AppTextLabel& label,
                int16_t x,
                int16_t y,
                int16_t w,
                int16_t h,
                uint8_t scale,
                const char* text,
                touchgfx::colortype fg,
                touchgfx::colortype bg,
                AppTextLabel::Align align = AppTextLabel::ALIGN_LEFT,
                bool opaqueBackground = true)
{
    label.setPosition(x, y, w, h);
    label.setScale(scale);
    label.setColors(fg, bg, opaqueBackground);
    label.setAlignment(align);
    label.setText(text);
}
}

TemplateView::TemplateView()
    : navPressedCallback(this, &TemplateView::onNavPressed),
      profilePressedCallback(this, &TemplateView::onProfilePressed),
      imageActionPressedCallback(this, &TemplateView::onImageActionPressed),
      mediaPressedCallback(this, &TemplateView::onMediaPressed),
      activeScreen(APP_UI_SCREEN_IMAGE),
      activeProfile(APP_UI_PROFILE_BALANCED),
      mediaPreviewGeneration(0U)
{
}

void TemplateView::setupScreen()
{
    setupStaticUi();
    setupNavigation();
    setupImagePage();
    setupMicPage();
    setupPerfPage();
    setupSettingsPage();
    setupMediaPage();
    setupDetails();
    refreshVisibility();
    refreshNavigation();
    refreshProfileButtons();
    invalidate();
}

void TemplateView::tearDownScreen()
{
}

void TemplateView::setupStaticUi()
{
    background.setPosition(0, 0, ScreenW, ScreenH);
    background.setColor(ColorBg);
    add(background);

    cameraPreviewKey.setPosition(CameraPreviewX, CameraPreviewY, CameraPreviewW, CameraPreviewH);
    cameraPreviewKey.setColor(rgb(255, 0, 255));
    cameraPreviewKey.setVisible(false);
    add(cameraPreviewKey);

    topBar.setPosition(0, 0, ScreenW, 56);
    topBar.setColor(ColorPanel);
    add(topBar);

    navPanel.setPosition(0, 56, NavW, 544);
    navPanel.setColor(ColorPanel2);
    add(navPanel);

    contentPanel.setPosition(192, 86, 560, 468);
    contentPanel.setColor(ColorPanel);
    add(contentPanel);

    detailPanel.setPosition(776, 86, 232, 468);
    detailPanel.setColor(ColorPanel2);
    add(detailPanel);

    imageHudPanel.setPosition(RightHudX, 72, RightHudW, 468);
    imageHudPanel.setColor(ColorPanel);
    add(imageHudPanel);

    imageBottomPanel.setPosition(CameraPreviewX, 548, CameraPreviewW, 42);
    imageBottomPanel.setColor(ColorPanel);
    add(imageBottomPanel);

    cameraFrame[0].setPosition(CameraPreviewX - 4, CameraPreviewY - 4, CameraPreviewW + 8, 3);
    cameraFrame[1].setPosition(CameraPreviewX - 4, CameraPreviewY + CameraPreviewH + 1, CameraPreviewW + 8, 3);
    cameraFrame[2].setPosition(CameraPreviewX - 4, CameraPreviewY - 4, 3, CameraPreviewH + 8);
    cameraFrame[3].setPosition(CameraPreviewX + CameraPreviewW + 1, CameraPreviewY - 4, 3, CameraPreviewH + 8);
    for (uint32_t i = 0U; i < CameraFrameCount; ++i)
    {
        cameraFrame[i].setColor(ColorLine);
        add(cameraFrame[i]);
    }

    logoImage.setBitmap(touchgfx::Bitmap(BITMAP_UI_LOGO_ID));
    logoImage.setPosition(24, 16, 80, 24);
    add(logoImage);

    setupLabel(titleLabel, 190, 10, 330, 32, 3, "声学成像", ColorText, ColorPanel, AppTextLabel::ALIGN_LEFT, false);
    add(titleLabel);

    setupLabel(modeLabel, 560, 16, 370, 24, 2, "Wide32 48k 标准", ColorCyan, ColorPanel, AppTextLabel::ALIGN_RIGHT, false);
    add(modeLabel);

    statusDot.setPosition(970, 20, 18, 18);
    statusDot.setColor(ColorAmber);
    add(statusDot);

    setupLabel(pageTitleLabel, 206, 100, 520, 28, 2, "声学热图", ColorText, ColorPanel);
    add(pageTitleLabel);
}

void TemplateView::setupNavigation()
{
    const char* labels[NavCount] = {"成像", "阵列", "性能", "设置", "媒体"};
    const uint16_t icons[NavCount] = {
        BITMAP_UI_IMAGE_ID,
        BITMAP_UI_MIC_ID,
        BITMAP_UI_PERF_ID,
        BITMAP_UI_SETTINGS_ID,
        BITMAP_UI_MEDIA_ID
    };

    for (uint32_t i = 0U; i < NavCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(82 + (i * 78));
        navButton[i].setPosition(18, y, 140, 58);
        navButton[i].setColor(ColorPanel);
        navButton[i].setBorderColor(ColorLine);
        navButton[i].setBorderSize(2);
        add(navButton[i]);

        navIcon[i].setBitmap(touchgfx::Bitmap(icons[i]));
        navIcon[i].setPosition(42, static_cast<int16_t>(y + 17), 24, 24);
        add(navIcon[i]);

        setupLabel(navLabel[i],
                   84,
                   static_cast<int16_t>(y + 17),
                   60,
                   24,
                   2,
                   labels[i],
                   ColorText,
                   ColorPanel,
                   AppTextLabel::ALIGN_CENTER);
        add(navLabel[i]);

        navTouch[i].setPosition(18, y, 140, 58);
        navTouch[i].setAction(navPressedCallback);
        add(navTouch[i]);
    }
}

void TemplateView::setupImagePage()
{
    heatMap.setPosition(CameraPreviewX, CameraPreviewY, CameraPreviewW, CameraPreviewH);
    heatMap.setColors(ColorPanel, ColorLine);
    heatMap.setOverlayMode(false);
    heatMap.setVisible(false);

    const uint16_t hudIcons[4] = {
        BITMAP_UI_PEAK_ID,
        BITMAP_UI_PCMD_ID,
        BITMAP_UI_CAMERA_ID,
        BITMAP_UI_BATTERY_ID
    };
    for (uint32_t i = 0U; i < 4U; ++i)
    {
        imageHudIcon[i].setBitmap(touchgfx::Bitmap(hudIcons[i]));
        imageHudIcon[i].setPosition(870, static_cast<int16_t>(98 + (i * 64)), 24, 24);
        add(imageHudIcon[i]);
    }

    setupLabel(imageHudLabel[0], 910, 88, 92, 22, 1, "声源", ColorMuted, ColorPanel);
    setupLabel(imageHudLabel[1], 910, 112, 92, 24, 2, "--", ColorText, ColorPanel);
    setupLabel(imageHudLabel[2], 910, 152, 92, 22, 1, "PCMD", ColorMuted, ColorPanel);
    setupLabel(imageHudLabel[3], 910, 176, 92, 24, 2, "等待", ColorAmber, ColorPanel);
    setupLabel(imageHudLabel[4], 910, 216, 92, 22, 1, "相机", ColorMuted, ColorPanel);
    setupLabel(imageHudLabel[5], 910, 240, 92, 24, 2, "等待", ColorAmber, ColorPanel);
    setupLabel(imageHudLabel[6], 910, 280, 92, 22, 1, "性能", ColorMuted, ColorPanel);
    setupLabel(imageHudLabel[7], 910, 304, 92, 24, 2, "-- FPS", ColorCyan, ColorPanel);
    for (uint32_t i = 0U; i < ImageHudCount; ++i)
    {
        add(imageHudLabel[i]);
    }

    const char* profileLabels[ProfileCount] = {"快", "标", "质"};
    const uint16_t profileIcons[ProfileCount] = {
        BITMAP_UI_FAST_ID,
        BITMAP_UI_STANDARD_ID,
        BITMAP_UI_QUALITY_ID
    };
    for (uint32_t i = 0U; i < ProfileCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(356 + (i * 46));
        imageProfileButton[i].setPosition(858, y, 136, 36);
        imageProfileButton[i].setColor(ColorPanel2);
        imageProfileButton[i].setBorderColor(ColorLine);
        imageProfileButton[i].setBorderSize(2);
        add(imageProfileButton[i]);

        imageProfileIcon[i].setBitmap(touchgfx::Bitmap(profileIcons[i]));
        imageProfileIcon[i].setPosition(876, static_cast<int16_t>(y + 6), 24, 24);
        add(imageProfileIcon[i]);

        setupLabel(imageProfileLabel[i],
                   916,
                   static_cast<int16_t>(y + 7),
                   62,
                   20,
                   1,
                   profileLabels[i],
                   ColorText,
                   ColorPanel2,
                   AppTextLabel::ALIGN_CENTER);
        add(imageProfileLabel[i]);

        imageProfileTouch[i].setPosition(858, y, 136, 36);
        imageProfileTouch[i].setAction(profilePressedCallback);
        add(imageProfileTouch[i]);
    }

    const char* actions[ImageActionCount] = {"截图", "录像"};
    const uint16_t actionIcons[ImageActionCount] = {
        BITMAP_UI_SNAPSHOT_ID,
        BITMAP_UI_RECORD_ID
    };
    for (uint32_t i = 0U; i < ImageActionCount; ++i)
    {
        const int16_t x = static_cast<int16_t>(858 + (i * 70));
        imageActionButton[i].setPosition(x, 504, 62, 30);
        imageActionButton[i].setColor(ColorPanel2);
        imageActionButton[i].setBorderColor(ColorLine);
        imageActionButton[i].setBorderSize(2);
        add(imageActionButton[i]);

        imageActionIcon[i].setBitmap(touchgfx::Bitmap(actionIcons[i]));
        imageActionIcon[i].setPosition(static_cast<int16_t>(x + 8), 507, 24, 24);
        add(imageActionIcon[i]);

        setupLabel(imageActionLabel[i],
                   static_cast<int16_t>(x + 28),
                   511,
                   28,
                   16,
                   1,
                   actions[i],
                   ColorText,
                   ColorPanel2,
                   AppTextLabel::ALIGN_CENTER);
        add(imageActionLabel[i]);

        imageActionTouch[i].setPosition(x, 504, 62, 30);
        imageActionTouch[i].setAction(imageActionPressedCallback);
        add(imageActionTouch[i]);
    }

    setupLabel(heatMetricLabel[0], 210, 558, 166, 22, 1, "方位 --", ColorText, ColorPanel);
    setupLabel(heatMetricLabel[1], 382, 558, 132, 22, 1, "俯仰 --", ColorText, ColorPanel);
    setupLabel(heatMetricLabel[2], 520, 558, 142, 22, 1, "质量 --", ColorCyan, ColorPanel);
    setupLabel(heatMetricLabel[3], 668, 558, 142, 22, 1, "FPS --", ColorAmber, ColorPanel);

    for (uint32_t i = 0U; i < 4U; ++i)
    {
        add(heatMetricLabel[i]);
    }
}

void TemplateView::setupMicPage()
{
    for (uint32_t i = 0U; i < MicCount; ++i)
    {
        const int16_t col = static_cast<int16_t>(i % 8U);
        const int16_t row = static_cast<int16_t>(i / 8U);
        const int16_t x = static_cast<int16_t>(206 + (col * 64));
        const int16_t y = static_cast<int16_t>(150 + (row * 62));

        micCell[i].setPosition(x,
                               y,
                               54,
                               46);
        micCell[i].setColor(rgb(27, 45, 50));
        micCell[i].setBorderColor(ColorLine);
        micCell[i].setBorderSize(2);
        add(micCell[i]);

        setupLabel(micValueLabel[i],
                   static_cast<int16_t>(x - 2),
                   static_cast<int16_t>(y + 14),
                   58,
                   18,
                   1,
                   "M00 --",
                   ColorText,
                   rgb(27, 45, 50),
                   AppTextLabel::ALIGN_CENTER);
        add(micValueLabel[i]);
    }

    setupLabel(micSummaryLabel[0], 206, 416, 220, 24, 1, "32 路阵列健康", ColorText, ColorPanel);
    setupLabel(micSummaryLabel[1], 206, 462, 170, 24, 1, "平均 --dBFS", ColorCyan, ColorPanel);
    setupLabel(micSummaryLabel[2], 400, 462, 170, 24, 1, "峰值 --dBFS", ColorAmber, ColorPanel);
    setupLabel(micSummaryLabel[3], 400, 416, 250, 24, 1, "PCMD 等待", ColorAmber, ColorPanel);

    for (uint32_t i = 0U; i < 4U; ++i)
    {
        add(micSummaryLabel[i]);
    }
}

void TemplateView::setupPerfPage()
{
    const char* labels[PerfCount] = {"预处理", "FFT", "GCC", "SRP", "总计"};

    for (uint32_t i = 0U; i < PerfCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(160 + (i * 58));
        setupLabel(perfLabel[i], 206, static_cast<int16_t>(y - 2), 190, 24, 1, labels[i], ColorText, ColorPanel);
        add(perfLabel[i]);

        perfTrack[i].setPosition(410, y, 278, 24);
        perfTrack[i].setColor(rgb(28, 39, 44));
        add(perfTrack[i]);

        perfFill[i].setPosition(410, y, 20, 24);
        perfFill[i].setColor(ColorCyan);
        add(perfFill[i]);
    }
}

void TemplateView::setupSettingsPage()
{
    setupLabel(settingsLabel[0], 206, 150, 500, 24, 1, "模式 Wide32 / 48 kHz", ColorText, ColorPanel);
    setupLabel(settingsLabel[1], 206, 194, 500, 24, 1, "频点策略 标准 B16", ColorCyan, ColorPanel);
    setupLabel(settingsLabel[2], 206, 238, 500, 24, 1, "麦克风对 标准 160", ColorText, ColorPanel);
    setupLabel(settingsLabel[3], 206, 282, 500, 24, 1, "搜索 9x9 粗搜 / Top3 精搜", ColorText, ColorPanel);
    setupLabel(settingsLabel[4], 206, 326, 500, 24, 1, "输入 等待真实采集", ColorAmber, ColorPanel);
    setupLabel(settingsLabel[5], 206, 370, 500, 24, 1, "高频近场 Core16/192k 暂停", ColorMuted, ColorPanel);
    setupLabel(settingsLabel[6], 206, 414, 500, 24, 1, "调试信息 默认关闭", ColorMuted, ColorPanel);

    for (uint32_t i = 0U; i < SettingsCount; ++i)
    {
        add(settingsLabel[i]);
    }
}

void TemplateView::setupMediaPage()
{
    const char* initial[MediaLabelCount] = {
        "SD 等待",
        "文件系统 未挂载",
        "空间 -- / -- MB",
        "截图 00000 BMP",
        "视频 00000 AVI",
        "录制 00:00 F0000 DROP0",
        "选择 无",
        "最近 无",
        "读取 0B 错误 0",
        "媒体空闲"
    };

    for (uint32_t i = 0U; i < MediaLabelCount; ++i)
    {
        setupLabel(mediaLabel[i],
                   206,
                   static_cast<int16_t>(142 + (i * 32)),
                   250,
                   24,
                   1,
                   initial[i],
                   (i < 2U) ? ColorCyan : ColorText,
                   ColorPanel);
        add(mediaLabel[i]);
    }

    mediaPreview.setPosition(470, 142, 260, 220);
    mediaPreview.setColors(rgb(9, 15, 18), ColorLine);
    add(mediaPreview);

    const char* actions[MediaActionCount] = {"截图", "录像", "下一个", "读取", "同步"};
    const uint16_t icons[MediaActionCount] = {
        BITMAP_UI_SNAPSHOT_ID,
        BITMAP_UI_RECORD_ID,
        BITMAP_UI_MEDIA_ID,
        BITMAP_UI_SD_ID,
        BITMAP_UI_SETTINGS_ID
    };
    for (uint32_t i = 0U; i < MediaActionCount; ++i)
    {
        const int16_t x = static_cast<int16_t>(206 + (i * 104));
        mediaButton[i].setPosition(x, 516, 88, 38);
        mediaButton[i].setColor(ColorPanel2);
        mediaButton[i].setBorderColor(ColorLine);
        mediaButton[i].setBorderSize(2);
        add(mediaButton[i]);

        mediaButtonIcon[i].setBitmap(touchgfx::Bitmap(icons[i]));
        mediaButtonIcon[i].setPosition(static_cast<int16_t>(x + 9), 523, 24, 24);
        add(mediaButtonIcon[i]);

        setupLabel(mediaButtonLabel[i],
                   static_cast<int16_t>(x + 36),
                   527,
                   48,
                   16,
                   1,
                   actions[i],
                   ColorText,
                   ColorPanel2,
                   AppTextLabel::ALIGN_CENTER);
        add(mediaButtonLabel[i]);

        mediaTouch[i].setPosition(x, 516, 88, 38);
        mediaTouch[i].setAction(mediaPressedCallback);
        add(mediaTouch[i]);
    }
}

void TemplateView::setupDetails()
{
    const char* initial[DetailCount] = {
        "电源 等待",
        "电量 --%",
        "电池 --.--V",
        "系统 --.--V",
        "IBAT +0.00A",
        "触摸 等待",
        "坐标 X0000 Y0000",
        "模式 标准",
        "帧号 000000",
        "UI 20.0FPS",
        "BQ 0x0000 PIN 00"
    };

    for (uint32_t i = 0U; i < DetailCount; ++i)
    {
        setupLabel(detailLabel[i],
                   792,
                   static_cast<int16_t>(98 + (i * 31)),
                   200,
                   23,
                   1,
                   initial[i],
                   (i == 0U) ? ColorCyan : ColorText,
                   ColorPanel2);
        add(detailLabel[i]);
    }

    const char* labels[ProfileCount] = {"快", "标", "质"};
    const uint16_t icons[ProfileCount] = {
        BITMAP_UI_FAST_ID,
        BITMAP_UI_STANDARD_ID,
        BITMAP_UI_QUALITY_ID
    };
    for (uint32_t i = 0U; i < ProfileCount; ++i)
    {
        const int16_t x = static_cast<int16_t>(792 + (i * 68));
        profileButton[i].setPosition(x, 532, 58, 30);
        profileButton[i].setColor(ColorPanel);
        profileButton[i].setBorderColor(ColorLine);
        profileButton[i].setBorderSize(2);
        add(profileButton[i]);

        profileIcon[i].setBitmap(touchgfx::Bitmap(icons[i]));
        profileIcon[i].setPosition(static_cast<int16_t>(x + 8), 535, 24, 24);
        add(profileIcon[i]);

        setupLabel(profileLabel[i], static_cast<int16_t>(x + 30), 539, 24, 16, 1, labels[i], ColorText, ColorPanel, AppTextLabel::ALIGN_CENTER);
        add(profileLabel[i]);

        profileTouch[i].setPosition(x, 532, 58, 30);
        profileTouch[i].setAction(profilePressedCallback);
        add(profileTouch[i]);
    }
}

void TemplateView::updateSnapshot(const AppUiSnapshot& snapshot)
{
    const uint8_t previousScreen = activeScreen;
    activeScreen = snapshot.activeScreen;
    activeProfile = snapshot.activeProfile;

    if (previousScreen != activeScreen)
    {
        refreshVisibility();
    }

    refreshNavigation();
    refreshProfileButtons();
    refreshDetails(snapshot);

    if (activeScreen == APP_UI_SCREEN_IMAGE)
    {
        refreshImagePage(snapshot);
    }
    else if (activeScreen == APP_UI_SCREEN_MICS)
    {
        refreshMicPage(snapshot);
    }
    else if (activeScreen == APP_UI_SCREEN_PERF)
    {
        refreshPerfPage(snapshot);
    }
    else if (activeScreen == APP_UI_SCREEN_SETTINGS)
    {
        refreshSettingsPage(snapshot);
    }
    else
    {
        refreshMediaPage(snapshot);
    }
}

void TemplateView::refreshVisibility()
{
    const bool imageVisible = (activeScreen == APP_UI_SCREEN_IMAGE);
    const bool micVisible = (activeScreen == APP_UI_SCREEN_MICS);
    const bool perfVisible = (activeScreen == APP_UI_SCREEN_PERF);
    const bool settingsVisible = (activeScreen == APP_UI_SCREEN_SETTINGS);
    const bool mediaVisible = (activeScreen == APP_UI_SCREEN_MEDIA);

    AppCameraDisplay_SetVisible(imageVisible ? 1U : 0U);

    cameraPreviewKey.setVisible(imageVisible);
    imageHudPanel.setVisible(imageVisible);
    imageBottomPanel.setVisible(imageVisible);
    imageHudPanel.invalidate();
    imageBottomPanel.invalidate();

    for (uint32_t i = 0U; i < CameraFrameCount; ++i)
    {
        cameraFrame[i].setVisible(imageVisible);
        cameraFrame[i].invalidate();
    }

    contentPanel.setVisible(!imageVisible);
    detailPanel.setVisible(!imageVisible);
    pageTitleLabel.setVisible(!imageVisible);
    contentPanel.invalidate();
    detailPanel.invalidate();
    pageTitleLabel.invalidate();

    for (uint32_t i = 0U; i < DetailCount; ++i)
    {
        detailLabel[i].setVisible(!imageVisible);
        detailLabel[i].invalidate();
    }
    for (uint32_t i = 0U; i < ProfileCount; ++i)
    {
        profileButton[i].setVisible(!imageVisible);
        profileTouch[i].setVisible(!imageVisible);
        profileLabel[i].setVisible(!imageVisible);
        profileIcon[i].setVisible(!imageVisible);
        profileButton[i].invalidate();
        profileTouch[i].invalidate();
        profileLabel[i].invalidate();
        profileIcon[i].invalidate();

        imageProfileButton[i].setVisible(imageVisible);
        imageProfileTouch[i].setVisible(imageVisible);
        imageProfileLabel[i].setVisible(imageVisible);
        imageProfileIcon[i].setVisible(imageVisible);
        imageProfileButton[i].invalidate();
        imageProfileTouch[i].invalidate();
        imageProfileLabel[i].invalidate();
        imageProfileIcon[i].invalidate();
    }

    heatMap.setOverlayMode(false);
    heatMap.setVisible(false);
    heatMap.invalidate();
    for (uint32_t i = 0U; i < 4U; ++i)
    {
        heatMetricLabel[i].setVisible(imageVisible);
        imageHudIcon[i].setVisible(imageVisible);
        heatMetricLabel[i].invalidate();
        imageHudIcon[i].invalidate();
    }
    for (uint32_t i = 0U; i < ImageHudCount; ++i)
    {
        imageHudLabel[i].setVisible(imageVisible);
        imageHudLabel[i].invalidate();
    }
    for (uint32_t i = 0U; i < ImageActionCount; ++i)
    {
        imageActionButton[i].setVisible(imageVisible);
        imageActionIcon[i].setVisible(imageVisible);
        imageActionTouch[i].setVisible(imageVisible);
        imageActionLabel[i].setVisible(imageVisible);
        imageActionButton[i].invalidate();
        imageActionIcon[i].invalidate();
        imageActionTouch[i].invalidate();
        imageActionLabel[i].invalidate();
    }

    for (uint32_t i = 0U; i < MicCount; ++i)
    {
        micCell[i].setVisible(micVisible);
        micValueLabel[i].setVisible(micVisible);
        micCell[i].invalidate();
        micValueLabel[i].invalidate();
    }
    for (uint32_t i = 0U; i < 4U; ++i)
    {
        micSummaryLabel[i].setVisible(micVisible);
        micSummaryLabel[i].invalidate();
    }

    for (uint32_t i = 0U; i < PerfCount; ++i)
    {
        perfLabel[i].setVisible(perfVisible);
        perfTrack[i].setVisible(perfVisible);
        perfFill[i].setVisible(perfVisible);
        perfLabel[i].invalidate();
        perfTrack[i].invalidate();
        perfFill[i].invalidate();
    }

    for (uint32_t i = 0U; i < SettingsCount; ++i)
    {
        settingsLabel[i].setVisible(settingsVisible);
        settingsLabel[i].invalidate();
    }

    for (uint32_t i = 0U; i < MediaLabelCount; ++i)
    {
        mediaLabel[i].setVisible(mediaVisible);
        mediaLabel[i].invalidate();
    }
    mediaPreview.setVisible(mediaVisible);
    mediaPreview.invalidate();
    for (uint32_t i = 0U; i < MediaActionCount; ++i)
    {
        mediaButton[i].setVisible(mediaVisible);
        mediaButtonIcon[i].setVisible(mediaVisible);
        mediaTouch[i].setVisible(mediaVisible);
        mediaButtonLabel[i].setVisible(mediaVisible);
        mediaButton[i].invalidate();
        mediaButtonIcon[i].invalidate();
        mediaTouch[i].invalidate();
        mediaButtonLabel[i].invalidate();
    }

    cameraPreviewKey.invalidate();
    invalidate();
}

void TemplateView::refreshNavigation()
{
    char text[40];
    (void)snprintf(text, sizeof(text), "N647 %s", screenName(activeScreen));
    pageTitleLabel.setText(text);

    for (uint32_t i = 0U; i < NavCount; ++i)
    {
        const bool selected = (i == activeScreen);
        const touchgfx::colortype buttonColor = selected ? rgb(24, 60, 62) : ColorPanel;
        const touchgfx::colortype borderColor = selected ? ColorCyan : ColorLine;
        const touchgfx::colortype textColor = selected ? ColorText : ColorMuted;

        navButton[i].setColor(buttonColor);
        navButton[i].setBorderColor(borderColor);
        navButton[i].invalidate();
        navLabel[i].setColors(textColor, buttonColor, true);
        navIcon[i].invalidate();
    }
}

void TemplateView::refreshProfileButtons()
{
    char mode[40];
    (void)snprintf(mode, sizeof(mode), "Wide32 48k %s", profileName(activeProfile));
    modeLabel.setText(mode);

    for (uint32_t i = 0U; i < ProfileCount; ++i)
    {
        const bool selected = (i == activeProfile);
        const touchgfx::colortype buttonColor = selected ? rgb(24, 60, 62) : ColorPanel;
        const touchgfx::colortype borderColor = selected ? ColorCyan : ColorLine;
        profileButton[i].setColor(buttonColor);
        profileButton[i].setBorderColor(borderColor);
        profileButton[i].invalidate();
        profileIcon[i].invalidate();
        profileLabel[i].setColors(selected ? ColorText : ColorMuted, buttonColor, true);

        imageProfileButton[i].setColor(buttonColor);
        imageProfileButton[i].setBorderColor(borderColor);
        imageProfileButton[i].invalidate();
        imageProfileIcon[i].invalidate();
        imageProfileLabel[i].setColors(selected ? ColorText : ColorMuted, buttonColor, true);
    }
}

void TemplateView::refreshDetails(const AppUiSnapshot& snapshot)
{
    char text[48];
    const bool powerReady = ((snapshot.powerFlags & APP_UI_POWER_FLAG_BQ_PRESENT) != 0U);
    const bool adcReady = ((snapshot.powerFlags & APP_UI_POWER_FLAG_ADC_VALID) != 0U);
    const bool powerFault = ((snapshot.powerState == APP_UI_POWER_STATE_FAULT) ||
                             (snapshot.powerState == APP_UI_POWER_STATE_UNDERVOLTAGE));

    statusDot.setColor(powerFault ? rgb(225, 91, 63) :
                       (adcReady ? rgb(82, 196, 126) :
                       (snapshot.touchReady ? rgb(109, 212, 186) : rgb(226, 172, 62))));
    statusDot.invalidate();

    (void)snprintf(text, sizeof(text), "电源 %s", powerStateName(snapshot.powerState));
    detailLabel[0].setText(text);

    if (powerReady && adcReady)
    {
        (void)snprintf(text, sizeof(text), "电量 %03u%%", snapshot.batteryPct);
        detailLabel[1].setText(text);
        (void)snprintf(text, sizeof(text), "电池 %lu.%02luV",
                       static_cast<unsigned long>(snapshot.batteryMv / 1000U),
                       static_cast<unsigned long>((snapshot.batteryMv % 1000U) / 10U));
        detailLabel[2].setText(text);
        (void)snprintf(text, sizeof(text), "系统 %lu.%02luV",
                       static_cast<unsigned long>(snapshot.systemMv / 1000U),
                       static_cast<unsigned long>((snapshot.systemMv % 1000U) / 10U));
        detailLabel[3].setText(text);
    }
    else
    {
        detailLabel[1].setText("电量 --%");
        detailLabel[2].setText("电池 --.--V");
        detailLabel[3].setText("系统 --.--V");
    }

    const int32_t currentMa = snapshot.batteryCurrentMa;
    const uint32_t absCurrentMa = (currentMa < 0) ? static_cast<uint32_t>(-currentMa) : static_cast<uint32_t>(currentMa);
    (void)snprintf(text, sizeof(text), "IBAT %c%lu.%02luA",
                   (currentMa < 0) ? '-' : '+',
                   static_cast<unsigned long>(absCurrentMa / 1000U),
                   static_cast<unsigned long>((absCurrentMa % 1000U) / 10U));
    detailLabel[4].setText(text);

    (void)snprintf(text, sizeof(text), "触摸 %s %s", touchName(snapshot.touchIc), snapshot.touchDown ? "按下" : "抬起");
    detailLabel[5].setText(text);
    (void)snprintf(text, sizeof(text), "坐标 X%04u Y%04u", snapshot.touchX, snapshot.touchY);
    detailLabel[6].setText(text);
    (void)snprintf(text, sizeof(text), "模式 %s", profileName(snapshot.activeProfile));
    detailLabel[7].setText(text);
    (void)snprintf(text, sizeof(text), "帧号 %06lu", static_cast<unsigned long>(snapshot.frameSeq));
    detailLabel[8].setText(text);
    (void)snprintf(text, sizeof(text), "UI %lu.%luFPS",
                   static_cast<unsigned long>(snapshot.uiFpsX10 / 10U),
                   static_cast<unsigned long>(snapshot.uiFpsX10 % 10U));
    detailLabel[9].setText(text);
    (void)snprintf(text, sizeof(text), "BQ 0x%04X PIN %02lX",
                   static_cast<unsigned int>(snapshot.chargerStatus),
                   static_cast<unsigned long>(snapshot.powerPinState & 0xFFU));
    detailLabel[10].setText(text);
}

void TemplateView::refreshImagePage(const AppUiSnapshot& snapshot)
{
    char text[64];
    const bool acousticRunning = ((snapshot.acousticFlags & APP_UI_ACOUSTIC_FLAG_RUNNING) != 0U);
    const bool acousticValid = ((snapshot.acousticFlags & APP_UI_ACOUSTIC_FLAG_VALID) != 0U);
    const bool acousticDegraded = ((snapshot.acousticFlags & APP_UI_ACOUSTIC_FLAG_AUTO_DEGRADED) != 0U);
    const bool pcmdLive = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_FRAME_VALID) != 0U);
    const bool pcmdRawValid = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_RAW_VALID) != 0U);
    const bool pcmdFault = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_RAW_FAULT) != 0U);
    const bool recording = ((snapshot.mediaFlags & APP_UI_MEDIA_FLAG_RECORDING) != 0U);

    (void)snprintf(text, sizeof(text), "Wide32 %s  T%+03d P%+03d Q%02u",
                   profileShortName(activeProfile),
                   snapshot.thetaDeg,
                   snapshot.phiDeg,
                   snapshot.qualityPct);
    modeLabel.setText(text);

    (void)snprintf(text, sizeof(text), "%+03d / %+03d", snapshot.thetaDeg, snapshot.phiDeg);
    imageHudLabel[1].setText(text);
    imageHudLabel[1].setColors(acousticValid ? ColorText : ColorMuted, ColorPanel);

    if (pcmdFault)
    {
        imageHudLabel[3].setText("异常");
        imageHudLabel[3].setColors(ColorRed, ColorPanel);
    }
    else if (pcmdRawValid && pcmdLive)
    {
        imageHudLabel[3].setText("正常");
        imageHudLabel[3].setColors(ColorGreen, ColorPanel);
    }
    else
    {
        imageHudLabel[3].setText("等待");
        imageHudLabel[3].setColors(ColorAmber, ColorPanel);
    }

    imageHudLabel[5].setText(acousticRunning ? "显示中" : "等待");
    imageHudLabel[5].setColors(acousticRunning ? ColorGreen : ColorAmber, ColorPanel);

    (void)snprintf(text, sizeof(text), "%lu.%lu FPS",
                   static_cast<unsigned long>(snapshot.uiFpsX10 / 10U),
                   static_cast<unsigned long>(snapshot.uiFpsX10 % 10U));
    imageHudLabel[7].setText(text);
    imageHudLabel[7].setColors((snapshot.uiFpsX10 >= 100U) ? ColorCyan : ColorAmber, ColorPanel);

    (void)snprintf(text, sizeof(text), "方位 %+03d", snapshot.thetaDeg);
    heatMetricLabel[0].setText(text);
    (void)snprintf(text, sizeof(text), "俯仰 %+03d", snapshot.phiDeg);
    heatMetricLabel[1].setText(text);
    (void)snprintf(text, sizeof(text), "质量 %02u%%", snapshot.qualityPct);
    heatMetricLabel[2].setText(text);
    (void)snprintf(text, sizeof(text), "FPS %lu.%lu",
                   static_cast<unsigned long>(snapshot.uiFpsX10 / 10U),
                   static_cast<unsigned long>(snapshot.uiFpsX10 % 10U));
    heatMetricLabel[3].setText(text);

    (void)snprintf(text, sizeof(text), "SRP %lu.%02lums",
                   static_cast<unsigned long>(snapshot.srpMsX100 / 100U),
                   static_cast<unsigned long>(snapshot.srpMsX100 % 100U));
    imageHudLabel[6].setText(acousticDegraded ? "性能 降级" : "性能");
    imageHudLabel[7].setText(text);

    imageActionButton[1].setBorderColor(recording ? ColorRed : ColorLine);
    imageActionButton[1].setColor(recording ? rgb(70, 28, 30) : ColorPanel2);
    imageActionButton[1].invalidate();
    imageActionLabel[1].setText(recording ? "停止" : "录像");
    imageActionLabel[1].setColors(ColorText, recording ? rgb(70, 28, 30) : ColorPanel2, true);
}

void TemplateView::refreshMicPage(const AppUiSnapshot& snapshot)
{
    int32_t dbSum = 0;
    int8_t peakDbfs = -90;
    char text[32];
    const bool pcmdLive = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_FRAME_VALID) != 0U);
    const bool pcmdStarted = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_STARTED) != 0U);
    const bool pcmdDebug = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_DEBUG_ENABLED) != 0U);
    const bool pcmdRawValid = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_RAW_VALID) != 0U);
    const bool pcmdRawFault = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_RAW_FAULT) != 0U);
    const bool pcmdHighFloor =
        ((snapshot.pcmdRawQualityFlags & APP_UI_PCMD_RAW_FLAG_HIGH_FLOOR) != 0U);

    for (uint32_t i = 0U; i < MicCount; ++i)
    {
        const uint8_t level = snapshot.micLevel[i];
        const int8_t dbfs = snapshot.micDbfs[i];
        const touchgfx::colortype color = levelColor(level);
        dbSum += static_cast<int32_t>(dbfs);
        if (dbfs > peakDbfs)
        {
            peakDbfs = dbfs;
        }
        micCell[i].setColor(color);
        (void)snprintf(text, sizeof(text), "M%02lu %d",
                       static_cast<unsigned long>(i + 1U),
                       static_cast<int>(dbfs));
        micValueLabel[i].setColors(rgb(235, 239, 232), color, true);
        micValueLabel[i].setText(text);
        micCell[i].invalidate();
        micValueLabel[i].invalidate();
    }

    (void)snprintf(text, sizeof(text), "平均 %lddBFS", static_cast<long>(dbSum / static_cast<int32_t>(MicCount)));
    micSummaryLabel[1].setText(text);
    (void)snprintf(text, sizeof(text), "峰值 %ddBFS", static_cast<int>(peakDbfs));
    micSummaryLabel[2].setText(text);

    micSummaryLabel[0].setText(pcmdRawValid ? "32 路 PCMD 原始音频" :
                               (pcmdRawFault ? "32 路原始音频异常" : "32 路原始音频等待"));
    if (pcmdDebug)
    {
        (void)snprintf(text, sizeof(text), "P%X C%X R%lX",
                       snapshot.pcmdDevicePresentMask,
                       snapshot.pcmdDeviceConfigOkMask,
                       static_cast<unsigned long>(snapshot.pcmdRawQualityFlags & 0xFFU));
        micSummaryLabel[3].setText(text);
    }
    else if (pcmdRawFault)
    {
        (void)snprintf(text, sizeof(text), "PCMD 削顶 %u.%u%%",
                       static_cast<unsigned int>(snapshot.pcmdRawRailPercentX10 / 10U),
                       static_cast<unsigned int>(snapshot.pcmdRawRailPercentX10 % 10U));
        micSummaryLabel[3].setText(text);
    }
    else if (pcmdHighFloor && !pcmdRawValid)
    {
        micSummaryLabel[3].setText("PCMD 底噪偏高");
    }
    else if (pcmdLive)
    {
        micSummaryLabel[3].setText(pcmdRawValid ? "PCMD 正常" : "PCMD 帧等待原始音频");
    }
    else if (pcmdStarted)
    {
        micSummaryLabel[3].setText("PCMD 原始音频等待");
    }
    else
    {
        micSummaryLabel[3].setText("PCMD 离线");
    }
}

void TemplateView::refreshPerfPage(const AppUiSnapshot& snapshot)
{
    const touchgfx::colortype fillColors[PerfCount] = {
        ColorCyan,
        rgb(83, 160, 204),
        ColorAmber,
        ColorRed,
        ColorGreen
    };

    for (uint32_t i = 0U; i < PerfCount; ++i)
    {
        const uint16_t width = static_cast<uint16_t>((390U * snapshot.perfLoad[i]) / 100U);
        perfTrack[i].invalidate();
        perfFill[i].setPosition(perfTrack[i].getX(), perfTrack[i].getY(), width, perfTrack[i].getHeight());
        perfFill[i].setColor(fillColors[i]);
        perfFill[i].invalidate();
    }

    if ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_DEBUG_ENABLED) != 0U)
    {
        char text[32];
        perfLabel[0].setText("采集");
        (void)snprintf(text, sizeof(text), "FPS %lu.%lu",
                       static_cast<unsigned long>(snapshot.pcmdFpsX10 / 10U),
                       static_cast<unsigned long>(snapshot.pcmdFpsX10 % 10U));
        perfLabel[1].setText(text);
        (void)snprintf(text, sizeof(text), "发布 %lu", static_cast<unsigned long>(snapshot.pcmdPublishedFrames % 1000U));
        perfLabel[2].setText(text);
        (void)snprintf(text, sizeof(text), "丢帧 %lu", static_cast<unsigned long>(snapshot.pcmdDroppedHalves % 1000U));
        perfLabel[3].setText(text);
        (void)snprintf(text, sizeof(text), "DMA2D %lu", static_cast<unsigned long>(snapshot.cameraDma2dCopyCount % 1000U));
        perfLabel[4].setText(text);
    }
    else
    {
        char text[32];
        formatCyclesM(text, sizeof(text), "预处理", snapshot.srpPreprocessCycles);
        perfLabel[0].setText(text);
        formatCyclesM(text, sizeof(text), "FFT", snapshot.srpFftCycles);
        perfLabel[1].setText(text);
        formatCyclesM(text, sizeof(text), "GCC", snapshot.srpGccCycles);
        perfLabel[2].setText(text);
        formatCyclesM(text, sizeof(text), "搜索", snapshot.srpCoarseCycles + snapshot.srpFineCycles);
        perfLabel[3].setText(text);
        formatCyclesM(text, sizeof(text), "总计", snapshot.srpTotalCycles);
        perfLabel[4].setText(text);
    }
}

void TemplateView::refreshSettingsPage(const AppUiSnapshot& snapshot)
{
    char text[48];
    const bool pcmdLive = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_FRAME_VALID) != 0U);
    const bool pcmdStarted = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_STARTED) != 0U);
    const bool pcmdDebug = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_DEBUG_ENABLED) != 0U);
    const bool pcmdRawValid = ((snapshot.pcmdFlags & APP_UI_PCMD_FLAG_RAW_VALID) != 0U);
    const bool acousticRunning = ((snapshot.acousticFlags & APP_UI_ACOUSTIC_FLAG_RUNNING) != 0U);
    const bool acousticValid = ((snapshot.acousticFlags & APP_UI_ACOUSTIC_FLAG_VALID) != 0U);
    const bool acousticDegraded = ((snapshot.acousticFlags & APP_UI_ACOUSTIC_FLAG_AUTO_DEGRADED) != 0U);

    (void)snprintf(text, sizeof(text), "麦克风对 %s %u",
                   profileName(snapshot.activeProfile),
                   (snapshot.activeProfile == APP_UI_PROFILE_FAST) ? 96U :
                   ((snapshot.activeProfile == APP_UI_PROFILE_QUALITY) ? 240U : 160U));
    settingsLabel[2].setText(text);

    settingsLabel[1].setText((snapshot.activeProfile == APP_UI_PROFILE_FAST) ? "频点策略 快速 B12" :
                             ((snapshot.activeProfile == APP_UI_PROFILE_QUALITY) ? "频点策略 高质量 B40" :
                              "频点策略 标准 B16"));
    settingsLabel[4].setText(acousticValid ? "输入 PCMD -> SRP 有效" :
                             (pcmdLive ? "输入 PCMD 帧等待 SRP" :
                             (pcmdRawValid ? "输入 原始音频等待帧" :
                             (pcmdStarted ? "输入 原始音频校验中" : "输入 等待采集"))));
    if (pcmdDebug)
    {
        (void)snprintf(text, sizeof(text), "PCMD 帧%lu 丢%lu 同步%lu",
                       static_cast<unsigned long>(snapshot.pcmdPublishedFrames),
                       static_cast<unsigned long>(snapshot.pcmdDroppedHalves),
                       static_cast<unsigned long>(snapshot.pcmdSyncMissCount));
        settingsLabel[6].setText(text);
    }
    else
    {
        if (acousticRunning)
        {
            (void)snprintf(text, sizeof(text), "%s 处理%lu 失败%lu",
                           acousticDegraded ? "SRP 降级" : "SRP 运行",
                           static_cast<unsigned long>(snapshot.acousticProcessedFrames % 1000U),
                           static_cast<unsigned long>(snapshot.acousticFailedFrames % 1000U));
            settingsLabel[6].setText(text);
        }
        else
        {
            settingsLabel[6].setText(pcmdRawValid ? "真实采集 原始音频正常" : "真实采集 等待原始音频");
        }
    }
}

void TemplateView::refreshMediaPage(const AppUiSnapshot& snapshot)
{
    char text[96];
    const bool sdReady = ((snapshot.mediaFlags & APP_UI_MEDIA_FLAG_SD_READY) != 0U);
    const bool mounted = ((snapshot.mediaFlags & APP_UI_MEDIA_FLAG_FS_MOUNTED) != 0U);
    const bool formatted = ((snapshot.mediaFlags & APP_UI_MEDIA_FLAG_FORMATTED) != 0U);
    const bool recording = ((snapshot.mediaFlags & APP_UI_MEDIA_FLAG_RECORDING) != 0U);
    const bool busy = ((snapshot.mediaFlags & APP_UI_MEDIA_FLAG_BUSY) != 0U);
    const uint32_t minutes = snapshot.mediaRecordSeconds / 60U;
    const uint32_t seconds = snapshot.mediaRecordSeconds % 60U;

    mediaPreview.setSource(snapshot.mediaPreviewPixels,
                           snapshot.mediaPreviewWidth,
                           snapshot.mediaPreviewHeight,
                           (snapshot.mediaPreviewValid != 0U));
    if (mediaPreviewGeneration != snapshot.mediaPreviewGeneration)
    {
        mediaPreviewGeneration = snapshot.mediaPreviewGeneration;
        mediaPreview.invalidate();
    }

    mediaLabel[0].setText(sdReady ? "SD 就绪" : "SD 等待");
    mediaLabel[0].setColors(sdReady ? ColorCyan : ColorAmber, ColorPanel);

    (void)snprintf(text, sizeof(text), "文件系统 %s%s", mounted ? "已挂载" : "未挂载", formatted ? " 已格式化" : "");
    mediaLabel[1].setText(text);
    mediaLabel[1].setColors(mounted ? ColorCyan : ColorRed, ColorPanel);

    (void)snprintf(text, sizeof(text), "空间 %lu / %lu MB",
                   static_cast<unsigned long>(snapshot.mediaFreeMb),
                   static_cast<unsigned long>(snapshot.mediaTotalMb));
    mediaLabel[2].setText(text);

    (void)snprintf(text, sizeof(text), "截图 %05lu BMP",
                   static_cast<unsigned long>(snapshot.mediaScreenshots));
    mediaLabel[3].setText(text);

    (void)snprintf(text, sizeof(text), "视频 %05lu AVI",
                   static_cast<unsigned long>(snapshot.mediaVideos));
    mediaLabel[4].setText(text);

    (void)snprintf(text, sizeof(text), "录制 %02lu:%02lu F%04lu 丢%lu",
                   static_cast<unsigned long>(minutes),
                   static_cast<unsigned long>(seconds),
                   static_cast<unsigned long>(snapshot.mediaRecordFrames),
                   static_cast<unsigned long>(snapshot.mediaDroppedFrames));
    mediaLabel[5].setText(text);
    mediaLabel[5].setColors(recording ? ColorRed : ColorText, ColorPanel);

    if (snapshot.mediaSelectedFile[0] != '\0')
    {
        (void)snprintf(text, sizeof(text), "选择 %s %s",
                       selectedMediaName(snapshot.mediaSelectedType),
                       snapshot.mediaSelectedFile);
    }
    else
    {
        (void)snprintf(text, sizeof(text), "选择 %s", selectedMediaName(snapshot.mediaSelectedType));
    }
    mediaLabel[6].setText(text);

    if (snapshot.mediaLastFile[0] != '\0')
    {
        (void)snprintf(text, sizeof(text), "最近 %s", snapshot.mediaLastFile);
    }
    else
    {
        (void)snprintf(text, sizeof(text), "最近 无");
    }
    mediaLabel[7].setText(text);

    (void)snprintf(text, sizeof(text), "读取 %luB 错误 %lu",
                   static_cast<unsigned long>(snapshot.mediaLastReadBytes),
                   static_cast<unsigned long>(snapshot.mediaLastError));
    mediaLabel[8].setText(text);
    mediaLabel[8].setColors((snapshot.mediaLastError == 0U) ? ColorText : ColorRed, ColorPanel);

    if (snapshot.mediaPreviewValid != 0U)
    {
        (void)snprintf(text, sizeof(text), "预览 %s F%lu/%lu",
                       selectedMediaName(snapshot.mediaPreviewType),
                       static_cast<unsigned long>(snapshot.mediaPreviewFrameIndex),
                       static_cast<unsigned long>(snapshot.mediaPreviewFrameCount));
    }
    else
    {
        (void)snprintf(text, sizeof(text), "%s %s",
                       busy ? "忙" : "空闲",
                       mounted ? "媒体就绪" : "等待媒体");
    }
    mediaLabel[9].setText(text);

    mediaButtonLabel[1].setText(recording ? "停止" : "录像");

    for (uint32_t i = 0U; i < MediaActionCount; ++i)
    {
        const bool primary = ((i == 0U) || (i == 1U));
        const touchgfx::colortype buttonColor = busy ? rgb(34, 40, 42) :
                                                (recording && (i == 1U) ? rgb(70, 28, 30) :
                                                (primary ? rgb(24, 60, 62) : ColorPanel2));
        const touchgfx::colortype borderColor = busy ? ColorLine :
                                                (recording && (i == 1U) ? ColorRed :
                                                (primary ? ColorCyan : ColorLine));
        mediaButton[i].setColor(buttonColor);
        mediaButton[i].setBorderColor(borderColor);
        mediaButton[i].invalidate();
        mediaButtonIcon[i].invalidate();
        mediaButtonLabel[i].setColors(busy ? ColorMuted : ColorText, buttonColor, true);
    }
}

void TemplateView::onNavPressed(const touchgfx::AbstractButton& source)
{
    for (uint32_t i = 0U; i < NavCount; ++i)
    {
        if (&source == &navTouch[i])
        {
            presenter->selectScreen(static_cast<uint8_t>(i));
            return;
        }
    }
}

void TemplateView::onProfilePressed(const touchgfx::AbstractButton& source)
{
    for (uint32_t i = 0U; i < ProfileCount; ++i)
    {
        if ((&source == &profileTouch[i]) || (&source == &imageProfileTouch[i]))
        {
            presenter->selectProfile(static_cast<uint8_t>(i));
            return;
        }
    }
}

void TemplateView::onImageActionPressed(const touchgfx::AbstractButton& source)
{
    if (&source == &imageActionTouch[0])
    {
        presenter->requestScreenshot();
    }
    else if (&source == &imageActionTouch[1])
    {
        presenter->toggleRecording();
    }
}

void TemplateView::onMediaPressed(const touchgfx::AbstractButton& source)
{
    for (uint32_t i = 0U; i < MediaActionCount; ++i)
    {
        if (&source == &mediaTouch[i])
        {
            if (i == 0U)
            {
                presenter->requestScreenshot();
            }
            else if (i == 1U)
            {
                presenter->toggleRecording();
            }
            else if (i == 2U)
            {
                presenter->selectNextMedia();
            }
            else if (i == 3U)
            {
                presenter->readSelectedMedia();
            }
            else
            {
                presenter->refreshMedia();
            }
            return;
        }
    }
}
