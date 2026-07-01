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
        return "BQ";
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

static const uint32_t BQ_PIN_PROCHOT = 0x01U;
static const uint32_t BQ_PIN_CHRG_OK = 0x02U;
static const uint32_t BQ_PIN_OTG_VAP = 0x04U;
static const uint32_t BQ_PIN_CMPOUT = 0x08U;
static const uint32_t BQ_PIN_PG = 0x10U;

static const uint16_t BQ_FAULT_OTG_UVP = 0x0001U;
static const uint16_t BQ_FAULT_OTG_OVP = 0x0002U;
static const uint16_t BQ_FORCE_CONV_OFF = 0x0004U;
static const uint16_t BQ_FAULT_VSYS_UVP = 0x0008U;
static const uint16_t BQ_FAULT_SYSOVP = 0x0010U;
static const uint16_t BQ_FAULT_ACOC = 0x0020U;
static const uint16_t BQ_FAULT_BATOC = 0x0040U;
static const uint16_t BQ_FAULT_ACOV = 0x0080U;
static const uint16_t BQ_STAT_IN_OTG = 0x0100U;
static const uint16_t BQ_STAT_IN_PCHRG = 0x0200U;
static const uint16_t BQ_STAT_IN_FCHRG = 0x0400U;
static const uint16_t BQ_STAT_IN_IIN_DPM = 0x0800U;
static const uint16_t BQ_STAT_IN_VINDPM = 0x1000U;
static const uint16_t BQ_STAT_IN_VAP = 0x2000U;
static const uint16_t BQ_STAT_AC = 0x8000U;
static const uint16_t BQ_CHRG_OK_PULLDOWN_MASK = BQ_FORCE_CONV_OFF |
                                                 BQ_FAULT_VSYS_UVP |
                                                 BQ_FAULT_SYSOVP |
                                                 BQ_FAULT_ACOC |
                                                 BQ_FAULT_BATOC |
                                                 BQ_FAULT_ACOV;

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
      bqModePressedCallback(this, &TemplateView::onBqModePressed),
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
    const char* labels[NavCount] = {"IMAGE", "MICS", "PERF", "BQ", "MEDIA"};

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
    const char* initial[SettingsCount] = {
        "I2C WAIT INIT -- PROBE -- LAST -- ADC --",
        "ID M-- D-- UPD 000000 STATE --",
        "CHRG PIN0 AC0 FLT0 WANT LOW",
        "VBUS --.--V VBAT --.--V VSYS --.--V",
        "FAULT ACOV0 ACOC0 SYS0 VSYS0",
        "FAULT BATOC0 OTGUV0 OTGOV0 FORCE0",
        "PIN PRO0 CHG0 OTG0 CMP0 PG0 RAW 0x00",
        "MODE OTG0 VAP0 VINDPM0 IINDPM0",
        "OPT0 0x0000 LPOFF ADC 0x0000",
        "VCHG 0x0000 ICHG 0x0000 IIN 0x0000",
        "VSYSMIN 0x0000 OTGV 0x0000 OTGI 0x0000",
        "RAWADC BAT00 SYS00 BUS00 PSY00",
        "CMD TARGET -- STAT -- COUNT 0"
    };

    for (uint32_t i = 0U; i < SettingsCount; ++i)
    {
        setupLabel(settingsLabel[i],
                   190,
                   static_cast<int16_t>(132 + (i * 28)),
                   536,
                   24,
                   1,
                   initial[i],
                   (i == 0U) ? rgb(226, 172, 62) : rgb(216, 222, 216),
                   rgb(20, 23, 26));
        add(settingsLabel[i]);
    }

    const char* modeLabels[BqModeButtonCount] = {"LOW PWR", "NORMAL"};
    for (uint32_t i = 0U; i < BqModeButtonCount; ++i)
    {
        const int16_t x = static_cast<int16_t>(190 + (i * 150));
        bqModeButton[i].setPosition(x, 512, 132, 42);
        bqModeButton[i].setColor(rgb(33, 38, 41));
        bqModeButton[i].setBorderColor(rgb(58, 66, 68));
        bqModeButton[i].setBorderSize(2);
        add(bqModeButton[i]);

        setupLabel(bqModeLabel[i], static_cast<int16_t>(x + 8), 525, 116, 18, 1, modeLabels[i], rgb(223, 230, 224), rgb(33, 38, 41), AppTextLabel::ALIGN_CENTER);
        add(bqModeLabel[i]);

        bqModeTouch[i].setPosition(x, 512, 132, 42);
        bqModeTouch[i].setAction(bqModePressedCallback);
        add(bqModeTouch[i]);
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
    for (uint32_t i = 0U; i < BqModeButtonCount; ++i)
    {
        bqModeButton[i].setVisible(settingsVisible);
        bqModeTouch[i].setVisible(settingsVisible);
        bqModeLabel[i].setVisible(settingsVisible);
        bqModeButton[i].invalidate();
        bqModeTouch[i].invalidate();
        bqModeLabel[i].invalidate();
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
    if (activeScreen == APP_UI_SCREEN_SETTINGS)
    {
        (void)snprintf(text, sizeof(text), "BQ25730 DEBUG");
    }
    else
    {
        (void)snprintf(text, sizeof(text), "SRP PHAT %s", screenName(activeScreen));
    }
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
    const bool i2cOk = (powerReady &&
                        (snapshot.powerProbeStatus == 0) &&
                        (snapshot.powerLastI2cStatus == 0));
    const bool statAc = ((snapshot.chargerStatus & BQ_STAT_AC) != 0U);
    const bool chrgOkPin = ((snapshot.powerPinState & BQ_PIN_CHRG_OK) != 0U);
    const bool powerFault = ((snapshot.powerState == APP_UI_POWER_STATE_FAULT) ||
                             (snapshot.powerState == APP_UI_POWER_STATE_UNDERVOLTAGE));
    const touchgfx::colortype dotColor = powerFault ? rgb(225, 91, 63) :
                                         (i2cOk ? rgb(82, 196, 126) :
                                         (adcReady ? rgb(109, 212, 186) :
                                         (snapshot.touchReady ? rgb(109, 212, 186) : rgb(226, 172, 62))));

    statusDot.setColor(dotColor);
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
    (void)snprintf(text, sizeof(text), "BQ %s AC%u CHG%u 0x%04X",
                   i2cOk ? "OK" : "NO",
                   statAc ? 1U : 0U,
                   chrgOkPin ? 1U : 0U,
                   static_cast<unsigned int>(snapshot.chargerStatus));
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
    char text[96];
    const bool powerReady = ((snapshot.powerFlags & APP_UI_POWER_FLAG_BQ_PRESENT) != 0U);
    const bool adcReady = ((snapshot.powerFlags & APP_UI_POWER_FLAG_ADC_VALID) != 0U);
    const bool i2cOk = (powerReady &&
                        (snapshot.powerProbeStatus == 0) &&
                        (snapshot.powerLastI2cStatus == 0));
    const uint16_t chargerStatus = snapshot.chargerStatus;
    const bool statAc = ((chargerStatus & BQ_STAT_AC) != 0U);
    const bool chrgOkPin = ((snapshot.powerPinState & BQ_PIN_CHRG_OK) != 0U);
    const bool chrgOkFault = ((chargerStatus & BQ_CHRG_OK_PULLDOWN_MASK) != 0U);
    const bool chrgOkShouldRelease = (statAc && !chrgOkFault);
    const bool chrgOkMismatch = (chrgOkShouldRelease && !chrgOkPin);
    const uint32_t pin = snapshot.powerPinState;

    (void)snprintf(text, sizeof(text), "I2C %s INIT %ld PROBE %ld LAST %ld ADC %ld",
                   i2cOk ? "OK" : "DOWN",
                   static_cast<long>(snapshot.powerInitStatus),
                   static_cast<long>(snapshot.powerProbeStatus),
                   static_cast<long>(snapshot.powerLastI2cStatus),
                   static_cast<long>(snapshot.powerAdcStatus));
    settingsLabel[0].setText(text);
    settingsLabel[0].setColors(i2cOk ? rgb(161, 221, 206) : rgb(225, 145, 102), rgb(20, 23, 26));

    (void)snprintf(text, sizeof(text), "ID M%02X D%02X UPD %06lu STATE %s",
                   snapshot.bqManufacturerId,
                   snapshot.bqDeviceId,
                   static_cast<unsigned long>(snapshot.powerUpdateCount),
                   powerStateName(snapshot.powerState));
    settingsLabel[1].setText(text);

    (void)snprintf(text, sizeof(text), "CHRG PIN%u AC%u FLT%u WANT %s",
                   chrgOkPin ? 1U : 0U,
                   statAc ? 1U : 0U,
                   chrgOkFault ? 1U : 0U,
                   chrgOkShouldRelease ? "HI" : "LOW");
    settingsLabel[2].setText(text);
    settingsLabel[2].setColors(chrgOkMismatch ? rgb(225, 91, 63) :
                               (chrgOkShouldRelease ? rgb(161, 221, 206) : rgb(226, 172, 62)),
                               rgb(20, 23, 26));

    if (adcReady)
    {
        (void)snprintf(text, sizeof(text), "VBUS %lu.%02luV VBAT %lu.%02luV VSYS %lu.%02luV",
                       static_cast<unsigned long>(snapshot.inputVoltageMv / 1000U),
                       static_cast<unsigned long>((snapshot.inputVoltageMv % 1000U) / 10U),
                       static_cast<unsigned long>(snapshot.batteryMv / 1000U),
                       static_cast<unsigned long>((snapshot.batteryMv % 1000U) / 10U),
                       static_cast<unsigned long>(snapshot.systemMv / 1000U),
                       static_cast<unsigned long>((snapshot.systemMv % 1000U) / 10U));
    }
    else
    {
        (void)snprintf(text, sizeof(text), "VBUS --.--V VBAT --.--V VSYS --.--V ADC %ld",
                       static_cast<long>(snapshot.powerAdcStatus));
    }
    settingsLabel[3].setText(text);
    settingsLabel[3].setColors(adcReady ? rgb(216, 222, 216) : rgb(226, 172, 62), rgb(20, 23, 26));

    (void)snprintf(text, sizeof(text), "FAULT ACOV%u ACOC%u SYS%u VSYS%u",
                   ((chargerStatus & BQ_FAULT_ACOV) != 0U) ? 1U : 0U,
                   ((chargerStatus & BQ_FAULT_ACOC) != 0U) ? 1U : 0U,
                   ((chargerStatus & BQ_FAULT_SYSOVP) != 0U) ? 1U : 0U,
                   ((chargerStatus & BQ_FAULT_VSYS_UVP) != 0U) ? 1U : 0U);
    settingsLabel[4].setText(text);

    (void)snprintf(text, sizeof(text), "FAULT BATOC%u OTGUV%u OTGOV%u FORCE%u",
                   ((chargerStatus & BQ_FAULT_BATOC) != 0U) ? 1U : 0U,
                   ((chargerStatus & BQ_FAULT_OTG_UVP) != 0U) ? 1U : 0U,
                   ((chargerStatus & BQ_FAULT_OTG_OVP) != 0U) ? 1U : 0U,
                   ((chargerStatus & BQ_FORCE_CONV_OFF) != 0U) ? 1U : 0U);
    settingsLabel[5].setText(text);

    (void)snprintf(text, sizeof(text), "PIN PRO%u CHG%u OTG%u CMP%u PG%u RAW 0x%02lX",
                   ((pin & BQ_PIN_PROCHOT) != 0U) ? 1U : 0U,
                   ((pin & BQ_PIN_CHRG_OK) != 0U) ? 1U : 0U,
                   ((pin & BQ_PIN_OTG_VAP) != 0U) ? 1U : 0U,
                   ((pin & BQ_PIN_CMPOUT) != 0U) ? 1U : 0U,
                   ((pin & BQ_PIN_PG) != 0U) ? 1U : 0U,
                   static_cast<unsigned long>(pin & 0xFFU));
    settingsLabel[6].setText(text);

    (void)snprintf(text, sizeof(text), "MODE OTG%u VAP%u VINDPM%u IINDPM%u CHG%u%u",
                   ((chargerStatus & BQ_STAT_IN_OTG) != 0U) ? 1U : 0U,
                   ((chargerStatus & BQ_STAT_IN_VAP) != 0U) ? 1U : 0U,
                   ((chargerStatus & BQ_STAT_IN_VINDPM) != 0U) ? 1U : 0U,
                   ((chargerStatus & BQ_STAT_IN_IIN_DPM) != 0U) ? 1U : 0U,
                   ((chargerStatus & BQ_STAT_IN_PCHRG) != 0U) ? 1U : 0U,
                   ((chargerStatus & BQ_STAT_IN_FCHRG) != 0U) ? 1U : 0U);
    settingsLabel[7].setText(text);

    (void)snprintf(text, sizeof(text), "OPT0 0x%04X LP%s ADC 0x%04X",
                   static_cast<unsigned int>(snapshot.bqChargeOption0),
                   snapshot.bqLowPowerEnabled ? "ON" : "OFF",
                   static_cast<unsigned int>(snapshot.bqAdcOption));
    settingsLabel[8].setText(text);

    (void)snprintf(text, sizeof(text), "VCHG 0x%04X ICHG 0x%04X IIN 0x%04X",
                   static_cast<unsigned int>(snapshot.bqChargeVoltageReg),
                   static_cast<unsigned int>(snapshot.bqChargeCurrentReg),
                   static_cast<unsigned int>(snapshot.bqInputCurrentReg));
    settingsLabel[9].setText(text);

    (void)snprintf(text, sizeof(text), "VSYSMIN 0x%04X OTGV 0x%04X OTGI 0x%04X",
                   static_cast<unsigned int>(snapshot.bqVsysMinReg),
                   static_cast<unsigned int>(snapshot.bqOtgVoltageReg),
                   static_cast<unsigned int>(snapshot.bqOtgCurrentReg));
    settingsLabel[10].setText(text);

    (void)snprintf(text, sizeof(text), "RAWADC BAT%02X SYS%02X BUS%02X PSY%02X",
                   static_cast<unsigned int>(snapshot.bqAdcRawVbat),
                   static_cast<unsigned int>(snapshot.bqAdcRawVsys),
                   static_cast<unsigned int>(snapshot.bqAdcRawVbus),
                   static_cast<unsigned int>(snapshot.bqAdcRawPsys));
    settingsLabel[11].setText(text);

    (void)snprintf(text, sizeof(text), "CMD %s TARGET %s STAT %ld CNT %lu",
                   snapshot.bqLowPowerRequestPending ? "PEND" : "IDLE",
                   snapshot.bqLowPowerTarget ? "LOW" : "NORMAL",
                   static_cast<long>(snapshot.bqLowPowerStatus),
                   static_cast<unsigned long>(snapshot.bqLowPowerCommandCount));
    settingsLabel[12].setText(text);

    for (uint32_t i = 0U; i < BqModeButtonCount; ++i)
    {
        const bool pending = ((snapshot.bqLowPowerRequestPending != 0U) &&
                              (((i == 0U) && (snapshot.bqLowPowerTarget != 0U)) ||
                               ((i == 1U) && (snapshot.bqLowPowerTarget == 0U))));
        const bool selected = (i2cOk &&
                               (((i == 0U) && (snapshot.bqLowPowerEnabled != 0U)) ||
                                ((i == 1U) && (snapshot.bqLowPowerEnabled == 0U))));
        const touchgfx::colortype buttonColor = !i2cOk ? rgb(46, 48, 48) :
                                                (pending ? rgb(88, 68, 40) :
                                                (selected ? rgb(58, 82, 74) : rgb(33, 38, 41)));
        const touchgfx::colortype borderColor = !i2cOk ? rgb(72, 76, 76) :
                                                (pending ? rgb(226, 172, 62) :
                                                (selected ? rgb(109, 212, 186) : rgb(58, 66, 68)));

        bqModeButton[i].setColor(buttonColor);
        bqModeButton[i].setBorderColor(borderColor);
        bqModeButton[i].invalidate();
        bqModeLabel[i].setColors(!i2cOk ? rgb(166, 174, 170) : rgb(223, 230, 224), buttonColor, true);
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

void TemplateView::onBqModePressed(const touchgfx::AbstractButton& source)
{
    for (uint32_t i = 0U; i < BqModeButtonCount; ++i)
    {
        if (&source == &bqModeTouch[i])
        {
            presenter->requestLowPowerMode((i == 0U) ? 1U : 0U);
            return;
        }
    }
}
