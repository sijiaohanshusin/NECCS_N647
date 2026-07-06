# N647 Bring-Up Status

This file is the persistent handoff point for the N647 cleanup and debug loop.
Update it before and after each hardware run so later work does not rediscover
the same failures.

## Current Checkpoint

- Repository checkpoint: `3f084b08 wip: checkpoint n6 pcmd acoustic bringup`.
- Hardware reference for PCMD/SAI timing: `7e12d5d fix(pcmd3180): lock SDOUT edge timing`.
- Current target: `Wide32 @ 48 kHz`, 4 x PCMD3180, SAI1 A/B, 16 slots per bus,
  256 samples per audio frame.

## Known Bugs

- PCMD may show `PCMDOFFLINE` or all `-90 dBFS`.
- MICS values can move but not correlate with ambient sound or tapping, so raw
  TDM slot data is not yet proven to be real microphone audio.
- IMAGE page has previously been stable black, with no visible camera or heatmap.
- Bottom text has flickered, and an earlier run showed red/green moving stripes,
  likely from LTDC layer, framebuffer, cache, or overlay corruption.
- Touch has previously stopped responding while firmware kept running. Suspect
  ThreadX scheduling, shared I2C2 contention, or long background work.
- Earlier live diagnostics showed U4 `0x4F` config failure, SAI error `0x82`,
  and DMA error `0x04`.
- Release cold boot previously landed in BootROM. BOOT0/BOOT1 must be checked
  before blaming application code.
- AP0 connect with AP1/core-ID failure means SWD is physically present but the
  core debug AP is inaccessible; power-cycle/reset/debug-auth state first.

## Bring-Up Order

1. Clock, HAL tick, HyperRAM, LTDC, RGBLCD.
2. TouchGFX UI and touch sampling with no camera or PCMD stress.
3. Shared I2C2 lock/recovery path.
4. Camera preview and LTDC layer/color-key hole.
5. PCMD raw capture: I2C presence, register readback, SAI/DMA counters, raw slot
   AC/DC/dBFS.
6. Audio frame publishing only after PCMD raw capture is internally consistent.
7. Acoustic SRP service only after valid capture frames exist.
8. IMAGE overlay drawn through the camera display path, not as an opaque
   TouchGFX widget.

## Hard Rules

- Do not restore the old bare RGBLCD PCMD debug page.
- PCMD HAL must use the shared `AppI2C2` lock/recovery path.
- PCMD/SAI timing keeps the `7e12d5d` facts: SAI audio clock before SAI init,
  SAI RX on rising edge, circular linked-list GPDMA, SAI A master before SAI B,
  and PCMD config only after BCLK/FSYNC are stable.
- All SAI/GPDMA linked-list nodes and RX buffers must be in a real MPU
  non-cacheable region before DCache is enabled. A section name alone is not
  enough; without the MPU region GPDMA can read stale linked-list nodes and
  raise `HAL_DMA_ERROR_USE`.
- UI must remain responsive: TouchGFX priority stays above bringup, PCMD, and
  acoustic threads. Background threads must sleep/yield between retries.
- MICS page must show real dBFS derived from DC-removed raw slot samples, not a
  synthetic percentage.
- Acoustic SRP must not publish a convincing heatmap until the input frame source
  is real, valid PCMD capture.

## Debug Loop

Run from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\n647_debug_env.ps1 -CheckOnly
powershell -ExecutionPolicy Bypass -File .\tools\debug\debug_n647_ram.ps1 -Batch
```

At the `main()` breakpoint in RAM Debug, use `g_app_bringup_control_mask` to
isolate modules without rebuilding:

- `0x00`: UI/display/touch only. Camera, PCMD, acoustic, power poll, and media
  are skipped.
- `0x02`: UI + camera preview only.
- `0x06`: UI + camera + PCMD raw capture, with acoustic and media skipped.
- `0x22`: UI + camera preview with the IMX219 test pattern enabled.
- `0x1f`: default full product path.

If the target cannot halt normally:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\debug_n647_ram.ps1 -ConnectUnderReset -Batch
```

Classify target state when boot behavior is ambiguous:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\probe_n647_state.ps1
```

Only after RAM Debug passes:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\flash_n647_release.ps1 -BuildBundle -ResetAfter
```

## GDB Symbols To Check

- Boot/display: `g_app_boot_diag`, `g_rgblcd_init_stage`,
  `g_rgblcd_effective_panel_id`, `GPIOA->ODR`, `LTDC->GCR`, `LTDC_Layer1->CFBAR`,
  `LTDC_Layer2->CFBAR`.
- Camera: `g_app_camera_frame_count`, `g_app_camera_ltdc_swap_count`,
  `g_app_camera_display_flags`, `g_app_camera_ltdc_error_count`.
- Camera frame stats: `g_app_camera_frame_sample_addr`,
  `g_app_camera_frame_sample_seq`, `g_app_camera_frame_sample_min`,
  `g_app_camera_frame_sample_max`, `g_app_camera_frame_sample_avg`,
  `g_app_camera_frame_sample_non_dark_count`,
  `g_app_camera_frame_sample_change_count`,
  `g_app_camera_frame_sample_center`.
- IMX219 readback: `g_app_camera_imx219_readback_status`,
  `g_app_camera_imx219_readback_lane_mode`,
  `g_app_camera_imx219_readback_data_format0/1`,
  `g_app_camera_imx219_readback_test_pattern`,
  `g_app_camera_imx219_readback_stream`,
  `g_app_camera_imx219_readback_exck_freq`,
  `g_app_camera_imx219_readback_line_length`,
  `g_app_camera_imx219_readback_frame_length`,
  `g_app_camera_imx219_readback_exposure`,
  `g_app_camera_imx219_readback_analog_gain`,
  `g_app_camera_imx219_readback_digital_gain`,
  crop/output/binning readbacks, and PLL multiplier readbacks.
- Touch/I2C: `g_touch`, `g_app_i2c2_snapshot`,
  `g_app_i2c2_hal_restore_count`, `g_app_i2c2_hal_recover_request_count`.
- PCMD: `s_snapshot` in `app_pcmd_capture.c`, `s_bus_a_rx`, `s_bus_b_rx`,
  SAI half/full/error counters, device present/config/status masks.
- Acoustic: `s_snapshot` in `app_acoustic_service.c`, processed/skipped/failed
  counts, profile, status, fps, and heat validity.

## Acceptance Notes

- Static silence: most MIC channels should be `<= -50 dBFS`.
- Tap test: tapping one physical MIC must produce a clear rise in one expected
  raw bus/slot and logical MIC. If all slots move together or random unrelated
  slots move, do not debug SRP yet.
- If raw buffers contain many near-rail samples (`0x7fff`, `0x8000`) and the
  average floor is around `-10 dBFS`, treat the PCMD/SAI bus as alive but the
  PDM input side as not yet valid. Check digital MIC supply, PDMCLK reach,
  PDMDIN wiring/pull state, and MIC L/R edge selection before touching SRP.
- Camera: IMAGE page must show live camera before acoustic overlay is enabled.
- Touch: navigation must remain responsive while PCMD retries and acoustic
  service are running.

## Latest Validation

- 2026-07-06 checkpoint `3f084b08` saved the messy PCMD/acoustic bring-up
  state before refactoring.
- 2026-07-06 `n647_debug_env.ps1 -CheckOnly`: passed. CubeIDE OpenOCD, GDB,
  CubeProgrammer CLI, and external loader were found.
- 2026-07-06 Debug build after bring-up snapshot and PCMD raw gating changes:
  passed, `0 errors / 0 warnings`.
- 2026-07-06 `debug_n647_ram.ps1 -SkipBuild -Batch`: passed. ELF downloaded to
  `0x3400...` RAM and stopped at `main()` with PC in AXI SRAM.
- 2026-07-06 first SAI callback before MPU fix was
  `HAL_SAI_ErrorCallback`: SAI error `0x80`, GPDMA error `0x04`
  (`HAL_DMA_ERROR_USE`), no half/full callbacks. GDB showed the CPU-visible DMA
  linked-list node was populated, so the likely root cause was DCache coherency.
- 2026-07-06 restored `App_MPU_ConfigNonCacheable()` before I/D cache enable and
  kept `.noncacheable` covering `s_bus_a_rx`, `s_bus_b_rx`, and
  `Node_GPDMA1_Channel0/1`. Also removed the duplicate runtime DMA channel
  attribute call from the SAI MSP path.
- 2026-07-06 after the MPU fix, first SAI callback was
  `HAL_SAI_RxHalfCpltCallback`; GPDMA error was `0`.
- 2026-07-06 PCMD/SAI long run: `present=0xf`, `cfg=0xf`, `status=0xf`,
  `start_status=0`, SAI A/B half/full counters increased and SAI/DMA errors
  stayed `0`. PCMD registers read back as expected: U1/U3 slots `0..7`,
  U2/U4 slots `8..15`, `PWR=0x60`, `PDMCLK=0x40`, `GPO=0x41`, `IN/OUT=0xff`,
  `ASI_STS=0x48` (48 kHz, 256 BCLK/FSYNC ratio).
- 2026-07-06 current blocker moved to PDM/raw validity: raw buffers contain many
  near-rail samples (`0x7fff/0x8000`) and silence-like bench conditions measured
  roughly `peak=-6 dBFS`, `avg=-9 dBFS`. This is not believable microphone
  audio. Firmware now flags this as `RAW_FLAG_RAIL_FAULT`, keeps MICS diagnostic
  levels visible, and blocks publication of `AppAudioFrame_t` to SRP until raw
  rail fault clears.
- 2026-07-06 RAM Debug `RAW_GATE` breakpoint after the rail gate patch:
  `present=0xf`, `cfg=0xf`, `status=0xf`, `start_status=0`, SAI/DMA errors `0`,
  but `flags=0x37`, `rail=3171/8192`, `rail_x10=387` (38.7%), `peak=-6 dBFS`,
  `avg=-9 dBFS`, `published=0`, `latest=0`. This is the intended safe state:
  PCMD/SAI is online, while invalid raw MIC data is visible on MICS diagnostics
  but not allowed into `AppAudioFrame_t` or SRP.
- 2026-07-06 added `g_app_bringup_control_mask` and snapshot
  `control/skipped/active` masks. Default remains full path, but GDB can now run
  UI-only, UI+camera, and UI+camera+PCMD scenarios without editing source. Use
  this before chasing touch freeze, IMAGE black screen, or PCMD raw issues.
- 2026-07-06 UI+camera (`0x02`) RAM Debug: IMX219 probe/start succeeded
  (`chip=0x219`, camera init/start `0`, `flags=0x33f`), DCMIPP frame callback
  fired, `AppCameraDisplay_RequestSwap()` reached `swaps=1`, LTDC layer1/layer2
  were enabled with layer1 CFBAR `0x90400000` and layer2 CFBAR `0x90072000`.
  The captured preview buffer was almost constant dark RGB565 (`0x1082`), so
  the next display/camera split check is `0x22` IMX219 test pattern.
- 2026-07-06 UI+camera test pattern (`0x22`) RAM Debug: `test=1`,
  `flags=0x73f`, swap reached layer1 `0x90400000`, and framebuffer samples
  contained visible pattern colors (`0xffff`, `0xffe0`, `0x07e0`). UI layer2
  color-key hole was also correct: preview pixels at `0x90090180` and
  `0x90108400` were `0xf81f`, while an outside UI pixel was `0x18e4`. This
  means LTDC layer order and color-key hole are working in the test-pattern
  path; normal live camera black should be chased in sensor exposure/output
  configuration before changing the UI overlay path again.
- 2026-07-06 added permanent camera diagnostics: IMX219 readbacks after
  init/stream/test-pattern and 8x8 sampled framebuffer stats on each DCMIPP
  frame callback. Debug build passed with `0 errors / 0 warnings`.
- 2026-07-06 Release build after camera diagnostics also passed with
  `0 errors / 0 warnings`; postbuild regenerated `Binary/appli.hex`.
- 2026-07-06 RAM Debug after adding camera diagnostics was blocked by target
  connection, not by a firmware assertion: normal OpenOCD reported
  `unable to connect to the target`; connect-under-reset/probe then reported
  `STLINK V0J8S0 (API v0) VID:PID 0000:0000` and still timed out waiting for
  GDB port `3333`. Before the next RAM-debug attempt, power-cycle or replug the
  ST-LINK/target and re-run `n647_debug_env.ps1 -CheckOnly` followed by
  `debug_n647_ram.ps1 -ConnectUnderReset -SkipBuild -Batch`.
