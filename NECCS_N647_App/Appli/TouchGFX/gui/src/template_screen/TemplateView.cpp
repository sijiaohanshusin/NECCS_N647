#include <gui/template_screen/TemplateView.hpp>

#include <touchgfx/Color.hpp>

#include <stdio.h>

namespace
{
touchgfx::colortype rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return touchgfx::Color::getColorFromRGB(r, g, b);
}

const char* screenName(uint8_t screen)
{
    switch (screen)
    {
    case APP_UI_SCREEN_IMAGE:
        return "IMAGE";
    case APP_UI_SCREEN_MICS:
        return "MICS";
    case APP_UI_SCREEN_PERF:
        return "PERF";
    case APP_UI_SCREEN_SETTINGS:
        return "SET";
    case APP_UI_SCREEN_MEDIA:
        return "MEDIA";
    default:
        return "IMAGE";
    }
}

const char* profileName(uint8_t profile)
{
    switch (profile)
    {
    case APP_UI_PROFILE_FAST:
        return "FAST";
    case APP_UI_PROFILE_QUALITY:
        return "QUAL";
    default:
        return "BAL";
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
        return "NO IC";
    }
}

const char* powerStateName(uint8_t state)
{
    switch (state)
    {
    case APP_UI_POWER_STATE_IDLE:
        return "IDLE";
    case APP_UI_POWER_STATE_CHARGING:
        return "CHG";
    case APP_UI_POWER_STATE_DISCHARGING:
        return "DISCHG";
    case APP_UI_POWER_STATE_OTG:
        return "OTG";
    case APP_UI_POWER_STATE_UNDERVOLTAGE:
        return "LOW";
    case APP_UI_POWER_STATE_FAULT:
        return "FAULT";
    default:
        return "WAIT";
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
        return "NONE";
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

void setupLabel(AppTextLabel& label,
                int16_t x,
                int16_t y,
                int16_t w,
                int16_t h,
                uint8_t scale,
                const char* text,
                touchgfx::colortype fg,
                touchgfx::colortype bg,
                AppTextLabel::Align align = AppTextLabel::ALIGN_LEFT)
{
    label.setPosition(x, y, w, h);
    label.setScale(scale);
    label.setColors(fg, bg, true);
    label.setAlignment(align);
    label.setText(text);
}
}

TemplateView::TemplateView()
    : navPressedCallback(this, &TemplateView::onNavPressed),
      profilePressedCallback(this, &TemplateView::onProfilePressed),
      mediaPressedCallback(this, &TemplateView::onMediaPressed),
      activeScreen(APP_UI_SCREEN_IMAGE),
      activeProfile(APP_UI_PROFILE_BALANCED)
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
}

void TemplateView::tearDownScreen()
{
}

void TemplateView::setupStaticUi()
{
    background.setPosition(0, 0, 1024, 600);
    background.setColor(rgb(15, 17, 19));
    add(background);

    topBar.setPosition(0, 0, 1024, 64);
    topBar.setColor(rgb(26, 30, 32));
    add(topBar);

    navPanel.setPosition(0, 64, 148, 536);
    navPanel.setColor(rgb(22, 25, 28));
    add(navPanel);

    contentPanel.setPosition(164, 80, 596, 496);
    contentPanel.setColor(rgb(20, 23, 26));
    add(contentPanel);

    detailPanel.setPosition(776, 80, 232, 496);
    detailPanel.setColor(rgb(23, 26, 29));
    add(detailPanel);

    setupLabel(titleLabel, 22, 14, 470, 34, 3, "NECCS N647 ACOUSTIC", rgb(238, 242, 236), rgb(26, 30, 32));
    add(titleLabel);

    setupLabel(modeLabel, 602, 18, 350, 28, 2, "WIDE32 48K BAL", rgb(161, 221, 206), rgb(26, 30, 32), AppTextLabel::ALIGN_RIGHT);
    add(modeLabel);

    statusDot.setPosition(976, 23, 18, 18);
    statusDot.setColor(rgb(226, 172, 62));
    add(statusDot);

    setupLabel(pageTitleLabel, 184, 96, 520, 28, 2, "SRP PHAT HEATMAP", rgb(235, 239, 232), rgb(20, 23, 26));
    add(pageTitleLabel);
}

void TemplateView::setupNavigation()
{
    const char* labels[NavCount] = {"IMAGE", "MICS", "PERF", "SET", "MEDIA"};

    for (uint32_t i = 0U; i < NavCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(88 + (i * 62));
        navButton[i].setPosition(18, y, 112, 46);
        navButton[i].setColor(rgb(29, 34, 37));
        navButton[i].setBorderColor(rgb(54, 62, 66));
        navButton[i].setBorderSize(2);
        add(navButton[i]);

        setupLabel(navLabel[i], 28, static_cast<int16_t>(y + 11), 92, 24, 2, labels[i], rgb(219, 226, 220), rgb(29, 34, 37), AppTextLabel::ALIGN_CENTER);
        add(navLabel[i]);

        navTouch[i].setPosition(18, y, 112, 46);
        navTouch[i].setAction(navPressedCallback);
        add(navTouch[i]);
    }
}

void TemplateView::setupImagePage()
{
    heatMap.setPosition(190, 140, 520, 322);
    heatMap.setColors(rgb(20, 23, 26), rgb(53, 60, 62));
    add(heatMap);

    setupLabel(heatMetricLabel[0], 190, 486, 122, 28, 2, "THETA 000", rgb(235, 239, 232), rgb(20, 23, 26));
    setupLabel(heatMetricLabel[1], 330, 486, 106, 28, 2, "PHI 00", rgb(235, 239, 232), rgb(20, 23, 26));
    setupLabel(heatMetricLabel[2], 470, 486, 88, 28, 2, "Q 00%", rgb(161, 221, 206), rgb(20, 23, 26));
    setupLabel(heatMetricLabel[3], 590, 486, 92, 28, 2, "C 00%", rgb(214, 170, 65), rgb(20, 23, 26));

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
        micCell[i].setPosition(static_cast<int16_t>(190 + (col * 64)),
                               static_cast<int16_t>(150 + (row * 66)),
                               52,
                               50);
        micCell[i].setColor(rgb(43, 73, 78));
        micCell[i].setBorderColor(rgb(57, 64, 65));
        micCell[i].setBorderSize(2);
        add(micCell[i]);
    }

    setupLabel(micSummaryLabel[0], 190, 430, 190, 28, 2, "32 MIC PLANAR", rgb(235, 239, 232), rgb(20, 23, 26));
    setupLabel(micSummaryLabel[1], 190, 466, 190, 28, 2, "AVG 00%", rgb(161, 221, 206), rgb(20, 23, 26));
    setupLabel(micSummaryLabel[2], 420, 466, 190, 28, 2, "PEAK 00%", rgb(214, 170, 65), rgb(20, 23, 26));
    setupLabel(micSummaryLabel[3], 420, 430, 220, 28, 2, "PCMD OFFLINE", rgb(225, 145, 102), rgb(20, 23, 26));

    for (uint32_t i = 0U; i < 4U; ++i)
    {
        add(micSummaryLabel[i]);
    }
}

void TemplateView::setupPerfPage()
{
    const char* labels[PerfCount] = {"PRE", "FFT", "GCC", "SRP", "UI"};

    for (uint32_t i = 0U; i < PerfCount; ++i)
    {
        const int16_t y = static_cast<int16_t>(160 + (i * 58));
        setupLabel(perfLabel[i], 198, static_cast<int16_t>(y - 2), 74, 28, 2, labels[i], rgb(235, 239, 232), rgb(20, 23, 26));
        add(perfLabel[i]);

        perfTrack[i].setPosition(288, y, 390, 24);
        perfTrack[i].setColor(rgb(35, 40, 43));
        add(perfTrack[i]);

        perfFill[i].setPosition(288, y, 20, 24);
        perfFill[i].setColor(rgb(83, 180, 156));
        add(perfFill[i]);
    }
}

void TemplateView::setupSettingsPage()
{
    setupLabel(settingsLabel[0], 190, 150, 420, 28, 2, "MODE WIDE32 48K", rgb(235, 239, 232), rgb(20, 23, 26));
    setupLabel(settingsLabel[1], 190, 194, 420, 28, 2, "NFFT 256 BINS 3..42", rgb(192, 199, 194), rgb(20, 23, 26));
    setupLabel(settingsLabel[2], 190, 238, 420, 28, 2, "PAIRS BAL 160", rgb(192, 199, 194), rgb(20, 23, 26));
    setupLabel(settingsLabel[3], 190, 282, 420, 28, 2, "GRID 9X9 TOP3 FINE", rgb(192, 199, 194), rgb(20, 23, 26));
    setupLabel(settingsLabel[4], 190, 326, 420, 28, 2, "SOURCE SYNTH", rgb(225, 145, 102), rgb(20, 23, 26));
    setupLabel(settingsLabel[5], 190, 370, 420, 28, 2, "CORE16 192K LOCKED", rgb(121, 132, 135), rgb(20, 23, 26));
    setupLabel(settingsLabel[6], 190, 414, 450, 28, 2, "REAL CAPTURE WAITS PCMD", rgb(121, 132, 135), rgb(20, 23, 26));

    for (uint32_t i = 0U; i < SettingsCount; ++i)
    {
        add(settingsLabel[i]);
    }
}

void TemplateView::setupMediaPage()
{
    const char* initial[MediaLabelCount] = {
        "SD WAIT",
        "FS UNMOUNTED",
        "SPACE -- / -- MB",
        "SCREEN 00000 BMP",
        "VIDEO 00000 AVI",
        "REC 00:00 F0000 DROP0",
        "SEL NONE",
        "LAST NONE",
        "READ 0B ERR 0",
        "FORMAT READY"
    };

    for (uint32_t i = 0U; i < MediaLabelCount; ++i)
    {
        setupLabel(mediaLabel[i],
                   190,
                   static_cast<int16_t>(142 + (i * 32)),
                   520,
                   24,
                   1,
                   initial[i],
                   (i < 2U) ? rgb(161, 221, 206) : rgb(216, 222, 216),
                   rgb(20, 23, 26));
        add(mediaLabel[i]);
    }

    const char* actions[MediaActionCount] = {"SHOT BMP", "REC AVI", "NEXT", "READ", "SYNC"};
    for (uint32_t i = 0U; i < MediaActionCount; ++i)
    {
        const int16_t x = static_cast<int16_t>(190 + (i * 104));
        mediaButton[i].setPosition(x, 520, 88, 34);
        mediaButton[i].setColor(rgb(33, 38, 41));
        mediaButton[i].setBorderColor(rgb(58, 66, 68));
        mediaButton[i].setBorderSize(2);
        add(mediaButton[i]);

        setupLabel(mediaButtonLabel[i], static_cast<int16_t>(x + 6), 529, 76, 16, 1, actions[i], rgb(223, 230, 224), rgb(33, 38, 41), AppTextLabel::ALIGN_CENTER);
        add(mediaButtonLabel[i]);

        mediaTouch[i].setPosition(x, 520, 88, 34);
        mediaTouch[i].setAction(mediaPressedCallback);
        add(mediaTouch[i]);
    }
}

void TemplateView::setupDetails()
{
    const char* initial[DetailCount] = {
        "POWER WAIT",
        "BAT --%",
        "VBAT --.--V",
        "VSYS --.--V",
        "IBAT +0.00A",
        "TOUCH NO IC",
        "POS X0000 Y0000",
        "PROFILE BAL",
        "FRAME 000000",
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
                   (i == 0U) ? rgb(161, 221, 206) : rgb(216, 222, 216),
                   rgb(23, 26, 29));
        add(detailLabel[i]);
    }

    const char* labels[ProfileCount] = {"FAST", "BAL", "QUAL"};
    for (uint32_t i = 0U; i < ProfileCount; ++i)
    {
        const int16_t x = static_cast<int16_t>(792 + (i * 68));
        profileButton[i].setPosition(x, 532, 58, 30);
        profileButton[i].setColor(rgb(33, 38, 41));
        profileButton[i].setBorderColor(rgb(58, 66, 68));
        profileButton[i].setBorderSize(2);
        add(profileButton[i]);

        setupLabel(profileLabel[i], static_cast<int16_t>(x + 4), 539, 50, 16, 1, labels[i], rgb(223, 230, 224), rgb(33, 38, 41), AppTextLabel::ALIGN_CENTER);
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

    heatMap.setVisible(imageVisible);
    heatMap.invalidate();
    for (uint32_t i = 0U; i < 4U; ++i)
    {
        heatMetricLabel[i].setVisible(imageVisible);
        heatMetricLabel[i].invalidate();
    }

    for (uint32_t i = 0U; i < MicCount; ++i)
    {
        micCell[i].setVisible(micVisible);
        micCell[i].invalidate();
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
    for (uint32_t i = 0U; i < MediaActionCount; ++i)
    {
        mediaButton[i].setVisible(mediaVisible);
        mediaTouch[i].setVisible(mediaVisible);
        mediaButtonLabel[i].setVisible(mediaVisible);
        mediaButton[i].invalidate();
        mediaTouch[i].invalidate();
        mediaButtonLabel[i].invalidate();
    }
}

void TemplateView::refreshNavigation()
{
    char text[40];
    (void)snprintf(text, sizeof(text), "SRP PHAT %s", screenName(activeScreen));
    pageTitleLabel.setText(text);

    for (uint32_t i = 0U; i < NavCount; ++i)
    {
        const bool selected = (i == activeScreen);
        const touchgfx::colortype buttonColor = selected ? rgb(51, 77, 76) : rgb(29, 34, 37);
        const touchgfx::colortype borderColor = selected ? rgb(109, 212, 186) : rgb(54, 62, 66);
        const touchgfx::colortype textColor = selected ? rgb(236, 242, 232) : rgb(188, 196, 192);

        navButton[i].setColor(buttonColor);
        navButton[i].setBorderColor(borderColor);
        navButton[i].invalidate();
        navLabel[i].setColors(textColor, buttonColor, true);
    }
}

void TemplateView::refreshProfileButtons()
{
    char mode[40];
    (void)snprintf(mode, sizeof(mode), "WIDE32 48K %s", profileName(activeProfile));
    modeLabel.setText(mode);

    for (uint32_t i = 0U; i < ProfileCount; ++i)
    {
        const bool selected = (i == activeProfile);
        const touchgfx::colortype buttonColor = selected ? rgb(58, 82, 74) : rgb(33, 38, 41);
        const touchgfx::colortype borderColor = selected ? rgb(109, 212, 186) : rgb(58, 66, 68);
        profileButton[i].setColor(buttonColor);
        profileButton[i].setBorderColor(borderColor);
        profileButton[i].invalidate();
        profileLabel[i].setColors(selected ? rgb(236, 242, 232) : rgb(198, 206, 200), buttonColor, true);
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

    (void)snprintf(text, sizeof(text), "POWER %s", powerStateName(snapshot.powerState));
    detailLabel[0].setText(text);

    if (powerReady && adcReady)
    {
        (void)snprintf(text, sizeof(text), "BAT %03u%%", snapshot.batteryPct);
        detailLabel[1].setText(text);
        (void)snprintf(text, sizeof(text), "VBAT %lu.%02luV",
                       static_cast<unsigned long>(snapshot.batteryMv / 1000U),
                       static_cast<unsigned long>((snapshot.batteryMv % 1000U) / 10U));
        detailLabel[2].setText(text);
        (void)snprintf(text, sizeof(text), "VSYS %lu.%02luV",
                       static_cast<unsigned long>(snapshot.systemMv / 1000U),
                       static_cast<unsigned long>((snapshot.systemMv % 1000U) / 10U));
        detailLabel[3].setText(text);
    }
    else
    {
        detailLabel[1].setText("BAT --%");
        detailLabel[2].setText("VBAT --.--V");
        detailLabel[3].setText("VSYS --.--V");
    }

    const int32_t currentMa = snapshot.batteryCurrentMa;
    const uint32_t absCurrentMa = (currentMa < 0) ? static_cast<uint32_t>(-currentMa) : static_cast<uint32_t>(currentMa);
    (void)snprintf(text, sizeof(text), "IBAT %c%lu.%02luA",
                   (currentMa < 0) ? '-' : '+',
                   static_cast<unsigned long>(absCurrentMa / 1000U),
                   static_cast<unsigned long>((absCurrentMa % 1000U) / 10U));
    detailLabel[4].setText(text);

    (void)snprintf(text, sizeof(text), "TOUCH %s %s", touchName(snapshot.touchIc), snapshot.touchDown ? "DOWN" : "UP");
    detailLabel[5].setText(text);
    (void)snprintf(text, sizeof(text), "POS X%04u Y%04u", snapshot.touchX, snapshot.touchY);
    detailLabel[6].setText(text);
    (void)snprintf(text, sizeof(text), "PROFILE %s", profileName(snapshot.activeProfile));
    detailLabel[7].setText(text);
    (void)snprintf(text, sizeof(text), "FRAME %06lu", static_cast<unsigned long>(snapshot.frameSeq));
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
    char text[32];

    heatMap.setValues(snapshot.heat, 81U, snapshot.peakIndex);

    (void)snprintf(text, sizeof(text), "THETA %+03d", snapshot.thetaDeg);
    heatMetricLabel[0].setText(text);
    (void)snprintf(text, sizeof(text), "PHI %+03d", snapshot.phiDeg);
    heatMetricLabel[1].setText(text);
    (void)snprintf(text, sizeof(text), "Q %02u%%", snapshot.qualityPct);
    heatMetricLabel[2].setText(text);
    (void)snprintf(text, sizeof(text), "C %02u%%", snapshot.contrastPct);
    heatMetricLabel[3].setText(text);
}

void TemplateView::refreshMicPage(const AppUiSnapshot& snapshot)
{
    uint32_t sum = 0U;
    uint8_t peak = 0U;
    char text[32];

    for (uint32_t i = 0U; i < MicCount; ++i)
    {
        const uint8_t level = snapshot.micLevel[i];
        sum += level;
        if (level > peak)
        {
            peak = level;
        }
        micCell[i].setColor(levelColor(level));
        micCell[i].invalidate();
    }

    (void)snprintf(text, sizeof(text), "AVG %02lu%%", static_cast<unsigned long>(sum / MicCount));
    micSummaryLabel[1].setText(text);
    (void)snprintf(text, sizeof(text), "PEAK %02u%%", peak);
    micSummaryLabel[2].setText(text);
}

void TemplateView::refreshPerfPage(const AppUiSnapshot& snapshot)
{
    const touchgfx::colortype fillColors[PerfCount] = {
        rgb(83, 180, 156),
        rgb(83, 160, 204),
        rgb(214, 170, 65),
        rgb(225, 91, 63),
        rgb(144, 184, 93)
    };

    for (uint32_t i = 0U; i < PerfCount; ++i)
    {
        const uint16_t width = static_cast<uint16_t>((390U * snapshot.perfLoad[i]) / 100U);
        perfTrack[i].invalidate();
        perfFill[i].setPosition(perfTrack[i].getX(), perfTrack[i].getY(), width, perfTrack[i].getHeight());
        perfFill[i].setColor(fillColors[i]);
        perfFill[i].invalidate();
    }
}

void TemplateView::refreshSettingsPage(const AppUiSnapshot& snapshot)
{
    char text[48];
    (void)snprintf(text, sizeof(text), "PAIRS %s %u",
                   profileName(snapshot.activeProfile),
                   (snapshot.activeProfile == APP_UI_PROFILE_FAST) ? 96U :
                   ((snapshot.activeProfile == APP_UI_PROFILE_QUALITY) ? 240U : 160U));
    settingsLabel[2].setText(text);
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

    mediaLabel[0].setText(sdReady ? "SD READY" : "SD WAIT");
    mediaLabel[0].setColors(sdReady ? rgb(161, 221, 206) : rgb(226, 172, 62), rgb(20, 23, 26));

    (void)snprintf(text, sizeof(text), "FS %s%s", mounted ? "MOUNTED" : "UNMOUNTED", formatted ? " FORMAT" : "");
    mediaLabel[1].setText(text);
    mediaLabel[1].setColors(mounted ? rgb(161, 221, 206) : rgb(225, 145, 102), rgb(20, 23, 26));

    (void)snprintf(text, sizeof(text), "SPACE %lu / %lu MB",
                   static_cast<unsigned long>(snapshot.mediaFreeMb),
                   static_cast<unsigned long>(snapshot.mediaTotalMb));
    mediaLabel[2].setText(text);

    (void)snprintf(text, sizeof(text), "SCREEN %05lu BMP",
                   static_cast<unsigned long>(snapshot.mediaScreenshots));
    mediaLabel[3].setText(text);

    (void)snprintf(text, sizeof(text), "VIDEO %05lu AVI",
                   static_cast<unsigned long>(snapshot.mediaVideos));
    mediaLabel[4].setText(text);

    (void)snprintf(text, sizeof(text), "REC %02lu:%02lu F%04lu DROP%lu",
                   static_cast<unsigned long>(minutes),
                   static_cast<unsigned long>(seconds),
                   static_cast<unsigned long>(snapshot.mediaRecordFrames),
                   static_cast<unsigned long>(snapshot.mediaDroppedFrames));
    mediaLabel[5].setText(text);
    mediaLabel[5].setColors(recording ? rgb(225, 91, 63) : rgb(216, 222, 216), rgb(20, 23, 26));

    if (snapshot.mediaSelectedFile[0] != '\0')
    {
        (void)snprintf(text, sizeof(text), "SEL %s %s",
                       selectedMediaName(snapshot.mediaSelectedType),
                       snapshot.mediaSelectedFile);
    }
    else
    {
        (void)snprintf(text, sizeof(text), "SEL %s", selectedMediaName(snapshot.mediaSelectedType));
    }
    mediaLabel[6].setText(text);

    if (snapshot.mediaLastFile[0] != '\0')
    {
        (void)snprintf(text, sizeof(text), "LAST %s", snapshot.mediaLastFile);
    }
    else
    {
        (void)snprintf(text, sizeof(text), "LAST NONE");
    }
    mediaLabel[7].setText(text);

    (void)snprintf(text, sizeof(text), "READ %luB ERR %lu",
                   static_cast<unsigned long>(snapshot.mediaLastReadBytes),
                   static_cast<unsigned long>(snapshot.mediaLastError));
    mediaLabel[8].setText(text);
    mediaLabel[8].setColors((snapshot.mediaLastError == 0U) ? rgb(216, 222, 216) : rgb(225, 145, 102), rgb(20, 23, 26));

    (void)snprintf(text, sizeof(text), "%s %s",
                   busy ? "BUSY" : "IDLE",
                   mounted ? "MEDIA READY" : "WAIT MEDIA");
    mediaLabel[9].setText(text);

    mediaButtonLabel[1].setText(recording ? "STOP" : "REC AVI");

    for (uint32_t i = 0U; i < MediaActionCount; ++i)
    {
        const bool primary = ((i == 0U) || (i == 1U));
        const touchgfx::colortype buttonColor = busy ? rgb(46, 48, 48) :
                                                (recording && (i == 1U) ? rgb(88, 45, 42) :
                                                (primary ? rgb(45, 67, 67) : rgb(33, 38, 41)));
        const touchgfx::colortype borderColor = busy ? rgb(72, 76, 76) :
                                                (recording && (i == 1U) ? rgb(225, 91, 63) :
                                                (primary ? rgb(109, 212, 186) : rgb(58, 66, 68)));
        mediaButton[i].setColor(buttonColor);
        mediaButton[i].setBorderColor(borderColor);
        mediaButton[i].invalidate();
        mediaButtonLabel[i].setColors(busy ? rgb(166, 174, 170) : rgb(223, 230, 224), buttonColor, true);
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
        if (&source == &profileTouch[i])
        {
            presenter->selectProfile(static_cast<uint8_t>(i));
            return;
        }
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
