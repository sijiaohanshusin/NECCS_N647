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

#ifndef MODEL_HPP
#define MODEL_HPP

#include <stdint.h>

class ModelListener;

enum AppUiScreen
{
    APP_UI_SCREEN_IMAGE = 0,
    APP_UI_SCREEN_MICS = 1,
    APP_UI_SCREEN_PERF = 2,
    APP_UI_SCREEN_SETTINGS = 3,
    APP_UI_SCREEN_MEDIA = 4,
    /* Boot/self-check page; entered only at startup, never via navigation. */
    APP_UI_SCREEN_BOOT = 5,
    /* Directional recording station (beamformer aim + WAV capture). */
    APP_UI_SCREEN_BEAM = 6
};

/* Boot checklist module states derived from the bring-up snapshot. */
enum AppUiBootModuleState
{
    APP_UI_BOOT_MODULE_PENDING = 0,
    APP_UI_BOOT_MODULE_READY = 1,
    APP_UI_BOOT_MODULE_FAILED = 2,
    APP_UI_BOOT_MODULE_SKIPPED = 3
};

enum AppUiProfile
{
    APP_UI_PROFILE_FAST = 0,
    APP_UI_PROFILE_BALANCED = 1,
    APP_UI_PROFILE_QUALITY = 2
};

enum AppUiPowerState
{
    APP_UI_POWER_STATE_UNKNOWN = 0,
    APP_UI_POWER_STATE_IDLE = 1,
    APP_UI_POWER_STATE_CHARGING = 2,
    APP_UI_POWER_STATE_DISCHARGING = 3,
    APP_UI_POWER_STATE_OTG = 4,
    APP_UI_POWER_STATE_UNDERVOLTAGE = 5,
    APP_UI_POWER_STATE_FAULT = 6
};

enum AppUiPowerFlag
{
    APP_UI_POWER_FLAG_BQ_PRESENT = 0x00000001UL,
    APP_UI_POWER_FLAG_ADC_VALID = 0x00000002UL,
    APP_UI_POWER_FLAG_UNDERVOLTAGE_RAW = 0x00000004UL,
    APP_UI_POWER_FLAG_UNDERVOLTAGE_CONFIRMED = 0x00000008UL,
    APP_UI_POWER_FLAG_CHARGER_FAULT = 0x00000010UL
};

enum AppUiMediaFlag
{
    APP_UI_MEDIA_FLAG_CARD_PRESENT = 0x00000001UL,
    APP_UI_MEDIA_FLAG_SD_READY = 0x00000002UL,
    APP_UI_MEDIA_FLAG_FS_MOUNTED = 0x00000004UL,
    APP_UI_MEDIA_FLAG_FORMATTED = 0x00000008UL,
    APP_UI_MEDIA_FLAG_RECORDING = 0x00000010UL,
    APP_UI_MEDIA_FLAG_BUSY = 0x00000020UL,
    APP_UI_MEDIA_FLAG_PREVIEW_VALID = 0x00000040UL,
    APP_UI_MEDIA_FLAG_PLAYING = 0x00000080UL
};

enum AppUiPcmdFlag
{
    APP_UI_PCMD_FLAG_INITIALIZED = 0x00000001UL,
    APP_UI_PCMD_FLAG_STARTED = 0x00000002UL,
    APP_UI_PCMD_FLAG_FRAME_VALID = 0x00000004UL,
    APP_UI_PCMD_FLAG_DEBUG_ENABLED = 0x00000008UL,
    APP_UI_PCMD_FLAG_RAW_VALID = 0x00000010UL,
    APP_UI_PCMD_FLAG_RAW_FAULT = 0x00000020UL,
    APP_UI_PCMD_FLAG_RECOVERING = 0x00000040UL
};

enum AppUiPcmdRawQualityFlag
{
    APP_UI_PCMD_RAW_FLAG_CONFIG_OK = 0x00000001UL,
    APP_UI_PCMD_RAW_FLAG_DMA_SYNC = 0x00000002UL,
    APP_UI_PCMD_RAW_FLAG_NONZERO = 0x00000004UL,
    APP_UI_PCMD_RAW_FLAG_LOW_NOISE = 0x00000008UL,
    APP_UI_PCMD_RAW_FLAG_RAIL_FAULT = 0x00000010UL,
    APP_UI_PCMD_RAW_FLAG_HIGH_FLOOR = 0x00000020UL
};

enum AppUiAiClass
{
    APP_UI_AI_LISTENING = 0,   /* no anomaly / below noise floor */
    APP_UI_AI_GAS_LEAK = 1,
    APP_UI_AI_BEARING = 2,
    APP_UI_AI_ARC = 3,
    APP_UI_AI_IMPACT = 4,
    APP_UI_AI_AMBIENT = 5
};

enum AppUiAcousticFlag
{
    APP_UI_ACOUSTIC_FLAG_INITIALIZED = 0x00000001UL,
    APP_UI_ACOUSTIC_FLAG_RUNNING = 0x00000002UL,
    APP_UI_ACOUSTIC_FLAG_VALID = 0x00000004UL,
    APP_UI_ACOUSTIC_FLAG_AUTO_DEGRADED = 0x00000008UL
};

struct AppUiSnapshot
{
    uint32_t frameSeq;
    uint32_t bringupEnabledMask;
    uint32_t bringupReadyMask;
    uint32_t bringupFailedMask;
    uint32_t bringupSkippedMask;
    uint32_t bootElapsedMs;
    uint32_t powerFlags;
    uint32_t batteryMv;
    uint32_t systemMv;
    uint32_t mediaFlags;
    uint32_t mediaLastError;
    uint32_t mediaScreenshots;
    uint32_t mediaVideos;
    uint32_t mediaRecordFrames;
    uint32_t mediaDroppedFrames;
    uint32_t mediaRecordSeconds;
    uint32_t mediaLastReadBytes;
    uint32_t mediaFreeMb;
    uint32_t mediaTotalMb;
    uint32_t mediaPreviewGeneration;
    uint32_t mediaPreviewFrameIndex;
    uint32_t mediaPreviewFrameCount;
    /* Gallery grid (8 slots per page, 176x99 RGB565 thumbs). */
    uint32_t mediaThumbGeneration;
    uint32_t mediaThumbTotal;
    uint32_t mediaThumbPage;
    uint32_t mediaThumbPageCount;
    const uint16_t* mediaThumbPixels[8];
    uint8_t mediaThumbUsed[8];
    uint8_t mediaThumbValid[8];
    uint8_t mediaThumbType[8];
    uint32_t mediaThumbIndex[8];
    uint32_t pcmdFlags;
    uint32_t pcmdPublishedFrames;
    uint32_t pcmdDroppedHalves;
    uint32_t pcmdSyncMissCount;
    uint32_t pcmdRawQualityFlags;
    uint32_t pcmdRawRailSampleCount;
    uint32_t pcmdRawTotalSampleCount;
    uint32_t pcmdWatchdogRestarts;
    /* Neural-ART sound classifier telemetry. */
    uint32_t npuInferences;
    uint16_t npuLatencyUs;
    uint8_t npuActive;
    uint32_t acousticFlags;
    uint32_t acousticInputSeq;
    uint32_t acousticOutputSeq;
    uint32_t acousticProcessedFrames;
    uint32_t acousticSkippedFrames;
    uint32_t acousticFailedFrames;
    uint32_t acousticDegradedCount;
    uint32_t acousticActiveChannelMask;
    uint32_t srpPreprocessCycles;
    uint32_t srpFftCycles;
    uint32_t srpGccCycles;
    uint32_t srpCoarseCycles;
    uint32_t srpFineCycles;
    uint32_t srpTotalCycles;
    uint32_t cameraSwapCount;
    uint32_t cameraOverlayDrawCount;
    uint32_t cameraDma2dCopyCount;
    uint32_t cameraDisplayErrorCount;
    uint32_t cameraDma2dErrorCode;
    const uint16_t* mediaPreviewPixels;
    int32_t batteryCurrentMa;
    int32_t acousticLastStatus;
    uint32_t powerPinState;
    uint16_t touchX;
    uint16_t touchY;
    uint16_t touchRawX;
    uint16_t touchRawY;
    uint16_t srpMsX100;
    uint16_t acousticPairCount;
    uint16_t uiFpsX10;
    uint16_t chargerStatus;
    uint16_t mediaPreviewWidth;
    uint16_t mediaPreviewHeight;
    uint16_t pcmdFpsX10;
    uint16_t pcmdRawRailPercentX10;
    int16_t thetaDeg;
    int16_t phiDeg;
    uint8_t activeScreen;
    uint8_t activeProfile;
    uint8_t powerState;
    uint8_t mediaSelectedType;
    uint8_t mediaPreviewValid;
    uint8_t mediaPreviewType;
    uint8_t pcmdDevicePresentMask;
    uint8_t pcmdDeviceConfigOkMask;
    uint8_t pcmdDeviceStatusOkMask;
    uint8_t pcmdDebugEnabled;
    uint8_t pcmdRawActiveSlotCount;
    uint8_t batteryPct;
    uint8_t touchReady;
    uint8_t touchDown;
    uint8_t touchIc;
    /* Touch bring-up diagnostics for the on-screen fault banner (the touch
     * failure itself blocks any touch-driven diagnosis). */
    uint8_t touchLastError;
    uint8_t touchHalStatus;
    uint16_t touchInitAttempts;
    uint8_t qualityPct;
    uint8_t contrastPct;
    uint8_t candCount;
    int16_t candTheta[3];
    int16_t candPhi[3];
    uint8_t candStrength[3];
    /* 1 while a detection is live or inside the hold window; the source
     * card shows "--" instead of noise angles when this drops to 0. */
    uint8_t sourceDisplayValid;
    uint32_t overlayDrawCycles;
    uint32_t triggerCount;
    uint16_t mediaEncodeMs;
    uint8_t triggerArmed;
    uint8_t trailEnabled;
    uint8_t spectrum[64];
    uint8_t spectrumPeakBin;
    /* Edge acoustic-signature classifier (rule-based scoring over live
     * spectrum features; presented as edge inference on the UI). */
    uint8_t aiClass;          /* AppUiAiClass */
    uint8_t aiConfidencePct;  /* 0 or 55..96 */
    uint8_t heatPalette;
    uint8_t acousticScene;
    /* Array mode: 0 = Wide32@48k, 1 = Core16@192k; switching = restart
     * in flight (params page shows a transitional label). */
    uint8_t arrayMode;
    uint8_t arraySwitching;
    int8_t acousticTempC;
    uint16_t acousticBandLoHz;
    uint16_t acousticBandHiHz;
    uint16_t acousticSpeedX10;
    /* Heat-field rendering parameters (settings page steppers). */
    int8_t fieldDbFloor;
    uint16_t fieldGammaX100;
    uint8_t fieldNoiseGateX100;
    uint8_t fieldSmoothPasses;
    uint8_t micLevel[32];
    int8_t micDbfs[32];
    int8_t pcmdRawPeakDbfs;
    int8_t pcmdRawAvgDbfs;
    uint8_t perfLoad[5];
    /* Directional recording (delay-and-sum beam over the array). */
    uint8_t beamActive;       /* beam mode engaged (aiming or recording) */
    uint8_t beamManual;       /* 1 = manual (tap/drag) steering, 0 = auto-track */
    uint8_t beamRecording;    /* WAV file being written */
    int16_t beamTheta;        /* applied steering, degrees */
    int16_t beamPhi;
    int8_t beamRmsDbfs;       /* beam output level */
    uint16_t beamSeconds;     /* recording duration */
    uint32_t beamClips;       /* AUDnnnnn.WAV count on card */
    /* Laser pointer / gimbal (hardware pending; laser relay pin is live). */
    uint8_t laserOn;
    char mediaLastFile[32];
    char mediaSelectedFile[32];
};

/**
 * The Model class defines the data model in the model-view-presenter paradigm.
 * The Model is a singular object used across all presenters. The currently active
 * presenter will have a pointer to the Model through deriving from ModelListener.
 *
 * The Model will typically contain UI state information that must be kept alive
 * through screen transitions. It also usually provides the interface to the rest
 * of the system (the backend). As such, the Model can receive events and data from
 * the backend and inform the current presenter of such events through the modelListener
 * pointer, which is automatically configured to point to the current presenter.
 * Conversely, the current presenter can trigger events in the backend through the Model.
 */
class Model
{
public:
    Model();

    /**
     * Sets the modelListener to point to the currently active presenter. Called automatically
     * when switching screen.
     */
    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    /**
     * This function will be called automatically every frame. Can be used to e.g. sample hardware
     * peripherals or read events from the surrounding system and inject events to the GUI through
     * the ModelListener interface.
     */
    void tick();

    void setActiveScreen(uint8_t screen);
    void setActiveProfile(uint8_t profile);
    void cycleHeatPalette();
    void cycleScene();
    void toggleArrayMode();
    void adjustTemperature(int8_t deltaC);
    void setBandHz(uint16_t loHz, uint16_t hiHz);
    /* param: 0=db_floor 1=gamma 2=noise_gate 3=smooth_passes; dir: -1/+1 */
    void adjustFieldParam(uint8_t param, int8_t dir);
    void toggleTrigger();
    void toggleTrail();
    void requestScreenshot();
    void toggleRecording();
    /* Directional recording: the beam engages whenever the beam page is
     * active (or a recording is running); no explicit mode toggle. */
    void toggleBeamRecording();
    void beamAutoTrack();
    /* Manual steering from the aim surface (camera-frame pixels 0..639/0..479). */
    void setBeamManualTargetPx(int16_t px, int16_t py);
    /* Laser pointer relay (gimbal skeleton; servos gated on new hardware). */
    void toggleLaser();
    void refreshMedia();
    void selectNextMedia();
    void readSelectedMedia();
    void playToggleMedia();
    /* Gallery: page navigation and tile selection (slot 0..7). */
    void requestThumbPage(uint32_t page);
    void selectMediaSlot(uint8_t slot);
    /* Popup menu overlays the camera window: hide the LTDC camera layer so
     * the dim scrim cannot blend with the color-key hole (purple smear). */
    void setMenuBlocksCamera(bool blocked);
    void rebootSystem();
    void powerOffSystem();

    const AppUiSnapshot& getSnapshot() const
    {
        return snapshot;
    }

protected:
    /**
     * Pointer to the currently active presenter.
     */
    ModelListener* modelListener;
    AppUiSnapshot snapshot;
    uint32_t tickCount;
    uint32_t bootTicks;
    uint32_t triggerCooldown;
    bool triggerArmed;
    bool triggerPrevHot;
    bool menuBlocksCamera;
};

#endif /* MODEL_HPP */
