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
- A software ST-LINK restart may not cut USB VBUS or target power. For remote
  recovery, use the H7 relay helper in
  `tools/usb_power_relay/h7_relay_controller/`. In the current lab wiring,
  relay `OFF` keeps the ST-LINK enumerated in Windows but drops CubeProgrammer
  target access-port count to `0`, so it is not a guaranteed ST-LINK USB VBUS
  cut. Add or rewire a second relay if full ST-LINK USB re-enumeration is
  required.

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
- `APP_PCMD_SDOUT_BCLK_MARGIN_FIX=1` is a two-sided timing fix: STM32 SAI RX
  uses rising-edge strobing and each PCMD3180 device config must set
  `invert_bclk=1`. Setting only the SAI edge produces alive-looking DMA data
  with false near-rail `0x7fff/0x8000` samples.
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
- `0x0c`: PCMD raw capture + acoustic service, with camera and media skipped.
- `0x22`: UI + camera preview with the IMX219 test pattern enabled.
- `0x1f`: default full product path.

If the target cannot halt normally:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\debug_n647_ram.ps1 -ConnectUnderReset -Batch
```

The local RAM OpenOCD cfg overrides the stock STM32N6 GDB attach hook in
connect-under-reset mode. The stock hook runs `reset init`, which times out on
this board even though a subsequent `halt` works; the override keeps attach to a
plain `halt`, then GDB performs the normal RAM load/reset sequence.

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
  average floor is around `-10 dBFS`, first verify the full
  `APP_PCMD_SDOUT_BCLK_MARGIN_FIX` pair: SAI RX rising edge plus PCMD
  `invert_bclk=1`. Only after the raw DMA halfwords match the `7e12d5d`
  low-amplitude two's-complement signature should PDM input wiring, MIC power,
  PDMCLK reach, or MIC L/R edge selection be suspected.
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
- 2026-07-06 historical failure signature before the BCLK polarity fix: raw
  buffers contained many near-rail samples (`0x7fff/0x8000`) and silence-like
  bench conditions measured roughly `peak=-6 dBFS`, `avg=-9 dBFS`. This was not
  believable microphone audio and was later traced to the missing PCMD-side
  `invert_bclk=1` setting, not to hardware.
- 2026-07-06 RAM Debug `RAW_GATE` breakpoint before the BCLK polarity fix:
  `present=0xf`, `cfg=0xf`, `status=0xf`, `start_status=0`, SAI/DMA errors `0`,
  but `flags=0x37`, `rail=3171/8192`, `rail_x10=387` (38.7%), `peak=-6 dBFS`,
  `avg=-9 dBFS`, `published=0`, `latest=0`. The rail gate correctly prevented
  invalid raw data from entering `AppAudioFrame_t` or SRP while the bus timing
  issue was still present.
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
- 2026-07-06 H7 relay controller on COM5 verified after flashing standalone H7
  relay firmware: `status`, `off`, `on`, `test`, and `cycle 5000` all returned
  CLI responses. `off` reports `relay state=USB_OFF ... pb1=1`, `on` reports
  `relay state=USB_ON ... pb1=0`. The Windows helper script now leaves DTR/RTS
  deasserted by default.
- 2026-07-06 relay OFF test: ST-LINK stayed visible in Windows and
  CubeProgrammer still listed the probe, but access-port count dropped from `3`
  to `0`. After ON, access-port count returned to `3`; AP0 and AP1 both
  connected under reset at 100 kHz.
- 2026-07-06 direct OpenOCD telnet probe showed the root of the RAM Debug
  failure: `reset init` timed out, but a following `halt` succeeded and stopped
  the core in BootROM at `pc=0x18003514`, `msp=0x341037a8`.
- 2026-07-06 patched `tools/debug/n647_openocd_ram.cfg` so
  connect-under-reset GDB attach uses `halt` instead of the stock STM32N6
  `reset init` hook. Validation passed:
  `debug_n647_ram.ps1 -ConnectUnderReset -SkipBuild -Batch -SwdKHz 100` loaded
  the Debug ELF to `0x3400...` and stopped at `main()`, then the same command at
  default 4 MHz also passed with PC `0x3400ebde` and SP `0x341fffe8`.
- 2026-07-06 PCMD branch `7e12d5d` was rechecked as the hardware-good baseline:
  four PCMD3180s ACK at `0x4c..0x4f`, slot readback is U1/U3 `0..7` and U2/U4
  `8..15`, SAI A/B DMA counters run without errors, and quiet levels are low.
  Do not re-open hardware suspicion unless this branch also fails.
- 2026-07-06 main branch PCMD config failure was fixed by making the shared I2C2
  lock reentrant and holding it across PCMD reset/config. RAM Debug showed
  `AppPcmdCapture_Start()` returning OK with `present/config/status=0xf`.
- 2026-07-06 main branch raw rail blocker was rechecked against the known-good
  `7e12d5d` PCMD branch. Baseline log
  `_debug_logs/gdb_pcmd_branch_7e12_20260706_203551.log` showed real quiet
  PCM: raw DMA halfwords near `0x0000/0xffff`, slot AC levels about `1..4`, and
  DC near zero. Main before the fix
  `_debug_logs/gdb_main_pcmd_process50_after_cfg_20260706_205336.log` still had
  persistent `0x7fff/0x8000` near-rail data after 50 half-buffers
  (`peak=-4 dBFS`, `avg=-7 dBFS`, rail fault).
- 2026-07-06 root cause: main had SAI RX on rising edge but did not copy the
  PCMD branch's matching PCMD-side `invert_bclk=1`. The earlier 15-bit decode
  theory was wrong and has been neutralized; `App_MicArray_DecodePcmdTdmSample()`
  now returns the normal 16-bit PCM sample unchanged.
- 2026-07-06 after enabling PCMD `invert_bclk=1` in `app_pcmd_capture.c` and
  aligning `pcmd3180.c` with the generic ASI config path, RAM Debug log
  `_debug_logs/gdb_main_pcmd_process50_bclkinv_20260706_205719.log` matched the
  PCMD branch data signature: `raw_audio_valid=1`, `latest_frame_valid=1`,
  `published_frames=50`, SAI/DMA errors `0`, `raw_rail_percent_x10=0`,
  `raw_peak_dbfs=-78`, `raw_avg_dbfs=-86`, slot AC levels about `1..5`, and raw
  DMA halfwords are small two's-complement PCM values instead of near-rail
  artifacts.
- 2026-07-06 acoustic-chain remote check did not reach PCMD/acoustic threads.
  `n647_debug_env.ps1 -CheckOnly` passed and Debug build passed. Standard
  `debug_n647_ram.ps1 -ConnectUnderReset -SkipBuild -Batch` still loads the ELF
  and stops at `main()`, but continuing from `main()` with
  `g_app_bringup_control_mask=0x0c` (PCMD raw + acoustic only) stalls before
  `MX_ThreadX_Init()`. External halt log
  `_debug_logs/gdb_xspi_fault_20260706_220729.log` shows the core at
  `main.c:275 -> MX_XSPI1_Init() -> HAL_XSPI_Init() ->
  XSPI_WaitFlagStateUntilTimeout()` waiting for `HAL_XSPI_FLAG_BUSY` to clear
  (`PC=0x34039f14`, `SP=0x341fff88`). `probe_n647_state.ps1 -ConnectUnderReset`
  reported the same RAM-debug PC. Therefore this run did not validate
  `AppPcmdCapture_GetLatestAudioFrame() -> AppAcousticService ->
  App_AcousticSrp_ProcessFrame() -> overlay`; the blocker is DEBUG
  HyperRAM/XSPI initialization before ThreadX.
- 2026-07-06 recovery limits during the acoustic-chain check: Windows
  `pnputil /restart-device "USB\\VID_0483&PID_3748\\6&E79D62A&0&1"` failed with
  `Access is denied` from the Codex shell, and the H7 relay helper exists but
  only COM1/COM2 were enumerated, not the documented COM5 relay. If the XSPI
  BUSY stall persists, use an administrator shell for ST-LINK restart or a real
  target power cycle before chasing PCMD/SRP.
- 2026-07-06 after a real remote 5V target power cycle, `n647_debug_env.ps1
  -CheckOnly` passed and `debug_n647_ram.ps1 -ConnectUnderReset -SkipBuild
  -Batch` again loaded the Debug ELF into AXI SRAM and stopped at `main()`.
- 2026-07-06 acoustic-chain clean checkpoint
  `_debug_logs/gdb_acoustic_snapshot_clean_20260706_223416.log` proved the first
  real PCMD frame reached SRP: the acoustic thread stopped at
  `app_acoustic_service.c:634` after `App_AcousticSrp_ProcessFrame()` returned
  `status=0`; `capture_frame` was `source=2` (`TDM_CAPTURE`), `format=2`
  (`PLANAR_F32`), `sample_rate=48000`, `channel_count=32`,
  `frame_len/samples_per_channel=256`, `channel_valid_mask=0xffffffff`, and
  drop/error counters were `0`. SRP context was `160` pairs, `40` active bins,
  and `129` grid points.
- 2026-07-06 the same acoustic checkpoint reported quiet/low-confidence SRP
  output: `snapshot.valid=0`, `quality_pct=0`, `contrast_pct=0`,
  candidate quality about `0.004`, and `s_vis_frame.valid=0`. This is the
  expected safe behavior for a quiet remote bench: do not show a strong heatmap
  until confidence rises with a real source.
- 2026-07-06 Debug/O0 performance warning from that checkpoint:
  `process_elapsed_ms=461` for one SRP frame. This is not Release performance,
  but it is far beyond the intended 20 FPS budget and must be remeasured in
  Release or with optimized Debug before enabling Balanced as a default UI
  overlay.
- 2026-07-06 attempted multi-frame acoustic GDB validation timed out because
  stale GDB/OpenOCD sessions left the target halted at the previous
  `app_acoustic_service.c:634` breakpoint, then later at `_exit(status=1)` with
  PSP near `touchgfx_byte_pool_buffer`. Treat this as a dirty debugger/target
  state unless it reproduces from a clean power cycle. Clean all stale
  `arm-none-eabi-gdb`/`openocd` processes and power-cycle before rerunning
  multi-frame checks.
- 2026-07-06 follow-up `probe_n647_state.ps1 -ConnectUnderReset` after cleaning
  stale debug processes still found `PC=0x34010cec`, `SP=0x341fffb0`,
  `LR=0x34005203`, `XPSR=0x1000003`, matching the previous HardFault handler
  address. The next acoustic validation run requires a real 5V power cycle,
  not just another GDB load.
- 2026-07-06 H7 relay recovery is now documented as COM5, 921600 8N1, but Codex
  currently gets `Access to the port 'COM5' is denied`, meaning another serial
  terminal or remote-control process owns the port. Close that port owner before
  expecting Codex to run
  `tools\usb_power_relay\h7_relay_controller\usb_power_cycle_h7.ps1 -Port COM5`.
- 2026-07-06 later `usb_power_cycle_h7.ps1 -ListPorts` from Codex enumerated
  only COM1/COM2, even though direct COM5 opens still reported access denied.
  If the user can manually cycle COM5 from another remote-control surface, that
  surface is probably hiding or owning the relay port from the Codex process.
- 2026-07-06 after another real target power cycle, clean RAM Debug passed again
  (`_debug_logs/gdb_n647_ram_20260706_231101.log`). Multi-frame acoustic
  validation then passed in
  `_debug_logs/gdb_acoustic_multiframe_clean_20260706_231148.log`: after
  skipping the first four SRP publish points, the fifth frame had `status=0`,
  `capture_frame.seq=3272`, `source=2` (`TDM_CAPTURE`), `format=2`
  (`PLANAR_F32`), `48000 Hz`, `32` channels, `256` samples/channel,
  `channel_valid_mask=0xffffffff`, and `error_count=0`.
- 2026-07-06 the same multi-frame run confirmed PCMD stayed healthy while SRP
  was running: present/config/status masks were all `0xf`,
  `raw_audio_valid=1`, `latest_frame_valid=1`, `published_frames=3805`,
  `latest_seq=3805`, `raw_quality_flags=0xf`, `raw_peak_dbfs=-78`,
  `raw_avg_dbfs=-83`, `raw_rail_percent_x10=0`, SAI/DMA error counters and
  error codes were all `0`, and logical MIC dBFS samples were mostly
  `-78..-90 dBFS` on the quiet remote bench.
- 2026-07-06 acoustic performance warning from the multi-frame run: the service
  had already auto-degraded to Fast (`pair_count=96`) and still measured about
  `280 ms` per SRP frame in Debug/O0 (`srp_ms_x100=28000`, `fps_x10=31`). The
  functional chain is now proven, but Release/optimized profiling and SRP
  reduction are required before claiming the 20 FPS UI target.
- 2026-07-06 COM5 relay became available from Codex after the port owner was
  released. `usb_power_cycle_h7.ps1 -Port COM5 -Action Status` returned
  `relay state=USB_ON cycles=2 pb1=0`, and `-Action Cycle -OffMs 5000`
  successfully power-cycled the target.
- 2026-07-06 full IMAGE first-swap checkpoint
  `_debug_logs/gdb_full_image_firstswap_20260706_232322.log`: camera init/start
  both returned `0`, IMX219 chip ID was `0x219`, camera state was `0x3bf`,
  LTDC layer1 was enabled at `0x90400000`, layer2 was enabled at `0x90072000`,
  swap count reached `1`, LTDC error count was `0`, and UI preview-hole pixels
  at `0x90090180`/`0x90108400` were `0xf81f`. Therefore the IMAGE black symptom
  is not explained by the UI layer covering the camera layer or by a missing
  color-key hole.
- 2026-07-06 full product-path SRP checkpoint
  `_debug_logs/gdb_full_path_srp5_20260706_232457.log`: camera, PCMD, and SRP
  ran concurrently. Camera had `425` frames and `425` LTDC swaps with `0` LTDC
  errors; layer1 CFBAR was `0x90500000`, layer2 CFBAR was `0x90072000`, and the
  color-key hole stayed `0xf81f`. PCMD had present/config/status masks `0xf`,
  `raw_audio_valid=1`, `latest_frame_valid=1`, `published_frames=4512`,
  `raw_peak_dbfs=-78`, `raw_avg_dbfs=-84`, rail percent `0`, and SAI/DMA errors
  `0`. SRP returned `status=0` on `capture_frame.seq=3915`, `TDM_CAPTURE`,
  `PLANAR_F32`, `48 kHz`, `32ch`, `256` samples.
- 2026-07-06 in that full product-path run, acoustic was still quiet/low
  confidence (`quality_pct=1`, `contrast_pct=1`, `s_vis_frame.valid=0`), so
  `g_app_camera_overlay_update_count` advanced but
  `g_app_camera_overlay_draw_count` stayed `0`. This is expected until a real
  sound source raises SRP confidence; it should not be treated as an overlay
  plumbing failure.
- 2026-07-06 remaining IMAGE issue: normal live camera samples are consistently
  dark rather than missing. Full-path stats around frame 425 were
  `min=0x0881`, `max=0x1082`, `avg=0x0fde`, `center=0x1081`,
  `non_dark_count=0`, while the previously verified IMX219 test-pattern path
  produced bright colors. Next camera work should focus on sensor exposure,
  output timing/format, lighting/lens state, or DCMIPP color processing for
  normal live mode, not LTDC layer ordering or TouchGFX color-key.
- 2026-07-06 PG4/CAM_LED_EN A-B check
  `_debug_logs/gdb_camera_led_ab_20260706_232811.log`: UI+camera-only run was
  sampled at the first swap, then GDB set `GPIOG->BSRR=0x10` so PG4 went high
  (`GPIOG->ODR` changed from `0x40` to `0x50`). After 30 more swaps the frame
  remained dark (`min=0x0862`, `max=0x1082`, `avg=0x0f3e`,
  `non_dark_count=0`). Therefore the live-camera darkness is not fixed by
  simply enabling CAM_LED_EN.
- 2026-07-06 current-code IMX219 test-pattern recheck
  `_debug_logs/gdb_camera_testpattern_check_20260706_232936.log`: UI+camera
  test-pattern mode (`g_app_bringup_control_mask=0x22`) produced bright frame
  data after 10 swaps: `min=0x0000`, `max=0xffff`, `avg=0x7fff`,
  `non_dark_count=56`, `center=0xf81f`, and preview-hole pixels stayed
  `0xf81f`. This confirms the RGB565 DCMIPP/LTDC/TouchGFX display path is
  healthy; the dark IMAGE symptom is specific to normal live sensor imagery.
- 2026-07-06 high exposure/gain live-camera A-B check
  `_debug_logs/gdb_camera_gain_ab_20260706_233228.log`: while stopped before
  `AppCameraIMX219_SetStream(1)`, GDB wrote exposure `0x0d00`, analog gain
  `0xff`, and digital gain `0x0800`; all writes returned `0`. After 30 swaps,
  live frame stats became bright (`min=0x14de`, `max=0xffff`, `avg=0xaa93`,
  `non_dark_count=64`). Therefore the normal IMAGE black symptom was caused by
  conservative IMX219 exposure/gain defaults, not by the display pipeline.
- 2026-07-06 firmware default changed to exposure `0x0d00`, analog gain `0xff`,
  and a gentler digital gain `0x0200`. Debug build passed. RAM Debug validation
  `_debug_logs/gdb_camera_default_gain_check_20260706_233452.log` showed normal
  UI+camera live frames after 30 swaps were no longer black:
  `min=0x0944`, `max=0xfc26`, `avg=0x4eca`, `center=0x2222`,
  `non_dark_count=63`, with IMX219 readback exposure/gain matching
  `0x0d00/0xff/0x0200` and preview-hole pixels still `0xf81f`.
- 2026-07-06 full product path after the IMX219 default fix
  `_debug_logs/gdb_full_path_srp5_after_gain_20260706_233624.log`: camera,
  PCMD, and SRP ran together with bright live frames. Camera reached `447`
  frames/swaps with `0` LTDC errors; framebuffer stats were `min=0x003f`,
  `max=0xfff1`, `avg=0x7439`, `non_dark_count=64`, `center=0x8cdf`, and IMX219
  readback stayed `0x0d00/0xff/0x0200`. PCMD stayed healthy with masks `0xf`,
  `raw_audio_valid=1`, `latest_frame_valid=1`, `published_frames=4739`,
  `raw_peak_dbfs=-84`, `raw_avg_dbfs=-89`, rail percent `0`, and SAI errors
  `0`. SRP returned `status=0` on a real TDM frame, but quiet-bench confidence
  was low (`quality_pct=5`, `contrast_pct=1`) and Debug/O0 processing was still
  slow (`445 ms`, Fast `96` pairs).
- 2026-07-06 UI overlay gate added after the quiet full-path run: TouchGFX keeps
  SRP validity and diagnostics visible, but `AppCameraDisplay_SetAcousticOverlay`
  is enabled only when the IMAGE page is active, SRP is valid, and
  `quality_pct >= 10`. The quiet run above (`quality_pct=5`) should therefore
  avoid drawing a fake heat spot while still proving that the acoustic service
  produced a valid diagnostic snapshot.
- 2026-07-06 overlay-gate RAM Debug validation
  `_debug_logs/gdb_overlay_gate_breakpoint_20260706_2351.log`: after H7 relay
  power-cycle and RAM load, full product path reached the fifth SRP checkpoint
  again. Camera had `445` frames/swaps, `0` LTDC errors, bright frame stats
  (`min=0x0865`, `max=0xffff`, `avg=0x6d48`, `non_dark_count=64`), and PCMD
  masks stayed `0xf` with `raw_audio_valid=1`, `latest_frame_valid=1`,
  `published_frames=4718`, `raw_peak_dbfs=-78`, `raw_avg_dbfs=-86`, SAI errors
  `0`. SRP processing returned `status=0` but quiet confidence was low
  (`snapshot.valid=0`, `quality_pct=2`, `contrast_pct=1`, `s_vis_frame.valid=0`,
  `351 ms`, Fast `96` pairs). The subsequent `Model.cpp:369` breakpoint showed
  `acoustic.valid=0`, `acoustic.quality_pct=1`, active screen IMAGE, and
  `acousticOverlayEnabled=0`; `g_app_camera_overlay_draw_count` remained `0`.
  This validates that quiet/invalid acoustic output no longer paints the camera
  overlay while the rest of the chain stays alive.
- 2026-07-06 after the overlay-gate change, `git diff --check` passed except
  for the expected `Model.cpp` LF-to-CRLF normalization notice. Debug and
  Release builds both completed with `0` errors. Release still emits the known
  generated makefile duplicate-recipe warnings for CMSIS-DSP/PCMD objects; this
  is build-metadata noise already present in the current project, not a new
  functional failure.
