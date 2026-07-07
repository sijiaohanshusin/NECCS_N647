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
- 2026-07-07 framebuffer compositor correction: LTDC layer1 is a `640x480`
  camera window at `(192,60)`, not a `1024x600` screen-sized buffer. Correctly
  composited dumps remove the earlier false color-block artifact and show the UI
  layer stable. Therefore any remaining IMAGE issue must be checked against the
  real `640x480` camera buffer first.
- 2026-07-07 full product live-camera recheck before the gain change:
  `_debug_logs/main_screen_full_composite_20260707.png` showed stable UI with
  the camera area full of random colored noise. GDB counters were healthy enough
  for display plumbing (`~166` frames/swaps, `0` LTDC errors, overlay draw `0`),
  while the dumped RGB565 camera frame covered `0x0000..0xffff` with `2981`
  unique values in the first `4096` pixels. This was not a TouchGFX/LTDC black
  screen.
- 2026-07-07 full product IMX219 test-pattern check:
  `_debug_logs/main_screen_test_pattern_full_composite_20260707.png` produced
  clean vertical color bars with only `16` unique RGB565 values across the test
  frame. This proves the IMX219 MIPI output, DCMIPP RAW10-to-RGB565 path, LTDC
  layer mapping, and UI color-key hole are fundamentally correct.
- 2026-07-07 current live-image conclusion: the previous `0xff` analog gain plus
  `0x0200` digital gain can turn a dark/covered remote bench into bright random
  sensor noise. Firmware default was changed to keep the long exposure
  (`0x0d00`) but reduce live gain to analog `0x80` and digital `0x0100`.
  RAM Debug with the rebuilt ELF read back `0x0d00/0x80/0x0100`, reached
  `167` camera frames/swaps with `0` LTDC errors, and the camera frame became a
  stable dark image (`min=0x0862`, `max=0x1882`, `avg=0x106b`,
  `unique_all=9`). The bench is still optically dark, but the rainbow-noise
  failure mode is removed; future work should use lighting/lens/ISP/AE, not
  max-gain amplification, to obtain usable live imagery.
- 2026-07-07 GDB-only analog-gain probe at `0xc0` with digital gain kept at
  `0x0100` ran for `382` camera frames/swaps and still produced the same stable
  dark floor (`0x1082`-dominant samples). This does not justify raising the
  firmware default above `0x80` in the current dark remote setup.
- 2026-07-07 acoustic overlay bring-up preview
  `_debug_logs/main_screen_overlay_preview_composite_20260707.png`: after H7
  relay power-cycle, Debug build, and full product RAM Debug, the IMAGE page
  showed a visible acoustic heat block over the camera window. GDB diagnostics:
  `ready_mask=0x7f7`, `failed_mask=0`, camera frames/swaps `391/391`,
  LTDC errors `0`, layer1 CFBAR `0x90655180`, layer2 CFBAR `0x90072000`,
  DCMIPP buffers `0x90400000/0x90500000`, compose buffer `0x90655180`,
  `g_app_camera_compose_count=391`, overlay updates `1042`, overlay draws
  `355`.
- 2026-07-07 overlay root cause/fix: drawing the heatmap directly onto the
  DCMIPP ping-pong buffers made the result unstable because the camera pipeline
  could reuse the displayed buffer and overwrite the overlay, leaving horizontal
  fragments or no obvious overlay. `AppCameraDisplay_RequestSwap()` now copies
  the completed camera frame into an independent `.EXTRAM` compose framebuffer,
  draws the acoustic overlay there, cleans cache for that buffer, and points
  LTDC layer1 at the compose buffer. The raw completed camera buffer no longer
  contains overlay colors; the LTDC compose dump does.
- 2026-07-07 current overlay behavior is intentionally a bring-up preview, not
  final confidence policy. `app_acoustic_service.c` and `Model.cpp` temporarily
  force a low-confidence preview overlay so the camera/overlay path can be
  verified in the quiet remote bench. Before product validation, restore/fade
  the confidence gate, calibrate yaw/pitch/FOV against a real sound source, and
  then optimize the copy/draw path for FPS.
- 2026-07-07 first Release performance probe
  `_debug_logs/perf_probe_release_build_20260707_retry.log`: Release build
  completed successfully with the current product-like compiler profile
  (`-Os`, `-mcpu=cortex-m55`, hard-float, `ARM_MATH_AUTOVECTORIZE`). The Release
  ELF is XIP-linked, not RAM-linked: `.isr_vector` at `0x70100400`, `.text` at
  `0x70100750`, `.data` at `0x34000000`, and `.EXTRAM` at `0x90600000`.
- 2026-07-07 Release bundle programming passed
  `_debug_logs/perf_probe_flash_release_20260707.log`: CubeProgrammer wrote and
  verified `_flash_images/n647_boot_bundle.hex` successfully. External-loader
  readback in `_debug_logs/perf_probe_release_flash_readback_20260707.log`
  showed a valid FSBL header at `0x70000000` (`324D5453`) and a valid App vector
  at `0x70100400` (`SP=0x34200000`, `PC=0x7010A125`).
- 2026-07-07 Release runtime sampling was blocked by boot mode, not by build or
  flashing. After CubeProgrammer reset and after a real H7 relay power-cycle,
  `probe_n647_state.ps1` still found the core in BootROM (`PC=0x18003514` then
  `PC=0x1800598c`), not in the `0x7010....` XIP App region. A GDB-side attempt
  to inspect the XIP memory map while halted in BootROM timed out, so it is not
  a safe path for performance sampling.
- 2026-07-07 no Release FPS table was produced for the `0x02`, `0x06`, or
  `0x1f` scenarios because the optimized Release App never ran. Do not use the
  previous Debug/O0 acoustic numbers (`~280..445 ms/frame`) as optimization
  ranking data; they remain functional-chain evidence only. To complete the
  first performance probe, either boot the board with the external-Flash strap
  (`BOOT0=GND`, `BOOT1=GND`) or add a separate RAM-linked optimized diagnostic
  configuration in a later, explicitly code-changing step.
- 2026-07-07 online boot-mode check for the remote Release blocker: ST's N6 boot
  material describes a DEV_BOOT path when BOOT1 is high and the project skill
  records the board rule as debug/development `BOOT0=GND, BOOT1=3.3V` versus
  external-Flash boot `BOOT0=GND, BOOT1=GND`. Useful references:
  [ST UM3234 boot ROM user manual](https://www.st.com/resource/en/user_manual/um3234-how-to-proceed-with-boot-rom-on-stm32n6-mcus-stmicroelectronics.pdf)
  and ST community notes on
  [STM32N6 boot ROM](https://community.st.com/stm32-mcus-60/stm32n6-boot-rom-explained-145756)
  /
  [running after flashing without changing BOOT1](https://community.st.com/stm32-mcus-products-25/how-to-run-program-on-stm32n6-after-flashing-without-changing-boot-1-jumper-153278).
- 2026-07-07 first optimized RAM performance probe completed with temporary
  `PerfRam` ELF (`Debug` RAM link, generated makefiles changed to `-Os`) because
  Release/XIP cannot be remotely booted with the current BOOT1 strap. This probe
  is closer to product than Debug/O0: I/D cache was on (`SCB->CCR=0x30201`),
  DWT was on (`DWT_CTRL=0x80000001`), `SystemCoreClock=600000000`, ThreadX and
  peripherals ran normally, and each scene started after an H7 relay power
  cycle. Probe helper/logs are under `_debug_logs/perfram_mask_*`.

  | Scene | Duration | Camera FPS | PCMD FPS by count | Acoustic FPS by count | SRP ms from cycles | SRP split | Notes |
  | --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
  | `0x03` POWER+camera | 20 s | 7.4 | off | off | n/a | n/a | Camera works but reports `last_error=-5`, HAL `0x10` (`PIPE1_OVR`); LTDC errors `0`. |
  | `0x05` POWER+PCMD | 15 s | off | 176.3 (`published_fps_x10=18750`) | off | n/a | n/a | PCMD healthy: masks `0xf`, SAI/DMA errors `0`, quiet `peak/avg=-84/-88 dBFS`. |
  | `0x07` POWER+camera+PCMD | 20 s | 7.5 | 36.9 | off | n/a | n/a | Camera/compose path alone already drags PCMD scheduling down; PCMD hardware still has `0` SAI/DMA errors. |
  | `0x0d` POWER+PCMD+acoustic | 20 s | off | 181.7 | 2.55 | 367.7 ms | coarse 56.8%, fine 36.1%, FFT+GCC 5.4% | SRP bottleneck is search/steering, not PCMD or FFT/GCC. |
  | `0x1f` full path | 20 s | 7.5 | 21.3 | 0.10 | 1738.0 ms | coarse 61.0%, fine 34.6%, FFT+GCC 3.8% | Camera/overlay/EXTRAM pressure makes SRP much worse; overlay drew `114` times, camera HAL `0x10`, PCMD errors still `0`. |

- 2026-07-07 performance conclusion: PCMD3180 + SAI is not the bottleneck and
  the hardware link is not suspect. The two real bottlenecks are:
  1. SRP search/steering (`coarse + fine`) dominates acoustic runtime even when
     camera is off. Next algorithm work should reduce evaluated grid/pair/bin
     work and move repeated steering work into LUTs/internal RAM/MVE-friendly
     loops before considering NPU.
  2. Camera compose/color-key/overlay path plus EXTRAM/AXI traffic reduces both
     camera FPS and PCMD publish rate. Before optimizing SRP to 20 FPS in the
     full product path, stabilize camera to its expected 15 FPS and remove
     CPU-heavy full-window color-key/compose work, likely via DMA2D/dirty-region
     or a less expensive LTDC/overlay strategy.
- 2026-07-07 measurement caveats: `0x02/0x06/0x22` are invalid for current
  hardware measurements because they omit the POWER bit. Use `0x03/0x07/0x23`
  instead. `pcmd_published_fps_x10` can remain `0` under heavy camera load even
  while `published_frames` advances, so count/duration is the more reliable
  comparison metric in overloaded scenes.
- 2026-07-07 first optimization implementation after the performance probe:
  `app_acoustic_srp.c` now places the real-time SRP workspace in a new
  `.SRP_FAST` linker section in internal RAM and leaves only the synthetic
  planar self-test buffer in `.EXTRAM`. The SRP coarse search no longer calls
  `arm_sin_cos_f32()` inside every frame/grid/pair accumulation; it prebuilds
  `sin_start/cos_start/sin_delta/cos_delta` phase steps during SRP init/profile
  setup. The fine search still rebuilds phase steps per selected top-k region,
  but accumulation now uses the same phase-step path, preparing the inner loop
  for later MVE work without changing the math.
- 2026-07-07 display-side first optimization implementation:
  `AppCameraDisplay_PrepareDisplayFrame()` now tries a guarded DMA2D RGB565
  full-frame copy from the completed DCMIPP buffer to the independent compose
  framebuffer. If DMA2D is busy, misconfigured, or times out, it aborts the
  transfer and falls back to the previous CPU `memcpy` path. New GDB-readable
  counters are `g_app_camera_dma2d_copy_count`,
  `g_app_camera_dma2d_fallback_count`, and
  `g_app_camera_dma2d_error_code`.
- 2026-07-07 build validation after first optimization implementation:
  `tools\build_n647_app.ps1 -Configuration Debug` passed, and
  `tools\build_n647_app.ps1 -Configuration Release` passed. Release map check:
  `.SRP_FAST` is at `0x3400cf00..0x340c7d20`, size `0xbae20` (~765 KB), in the
  current 2 MB internal RAM region. `.EXTRAM` remains at `0x90600000`, size
  `0x31f260`. Release ELF size after this step is `text=377210`,
  `data=52964`, `bss=4764968`. Runtime FPS/cycle validation still needs the
  N647 debug loop after power-cycle.
- 2026-07-07 RAM Debug smoke after first optimization implementation:
  `tools\debug\n647_debug_env.ps1 -CheckOnly` passed. The first
  `debug_n647_ram.ps1 -ConnectUnderReset -SkipBuild -Batch` attempt loaded the
  ELF but lost AP/core state after reset; after H7 relay power-cycle, the same
  command reached `main()` at `0x3400f352` with `SP=0x341fffe8`. This validates
  load/reset/entry for the optimized source in the then-current Debug RAM mode,
  but it is not a retained performance number. Current RAM Debug has since been
  fixed to `-Os + DEBUG + -g3`.
- 2026-07-07 DMA2D camera-compose breakpoint smoke:
  a temporary GDB run used `g_app_bringup_control_mask=0x03` and stopped after
  the 11th `AppCameraDisplay_RequestSwap` entry. Log:
  `_debug_logs/gdb_dma2d_break_20260707_095218.log`. Readback showed
  `control_mask=0x3`, `ready_mask=0x237`, `failed_mask=0`, `swap_count=10`,
  `compose_count=10`, `g_app_camera_dma2d_copy_count=10`,
  `g_app_camera_dma2d_fallback_count=0`,
  `g_app_camera_dma2d_error_code=0`, and `ltdc_error_count=3`. This confirms the
  guarded DMA2D copy path is being used for camera compose. The LTDC error count
  still needs follow-up correlation during a longer visual/FPS run.
- 2026-07-07 optimized RAM performance validation after `.SRP_FAST` +
  phase-step SRP + DMA2D compose:
  temporary `PerfRam` was rebuilt from the Debug RAM configuration with `-Os`
  and `DEBUG` kept enabled. Keeping `DEBUG` is intentional for RAM sampling:
  `main.c` only runs `sys_clock_config_debug()` and HyperRAM init in that path;
  a no-`DEBUG` RAM-linked Release-like ELF skipped those steps and lost
  halt/read access after running. GDB sampling used one connected session:
  load, set `g_app_bringup_control_mask`, run for the host-side duration, halt,
  then read snapshots. All runs had `SCB->CCR=0x30201`, `DWT_CTRL=0x80000001`,
  and `SystemCoreClock=600000000`.

  | Scene | Duration | Camera FPS / ms | PCMD FPS / ms by count | Acoustic processed FPS / ms by count | SRP cycles | SRP ms/FPS @600 MHz | Est. ms/FPS @800 MHz | SRP split | Notes |
  | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |
  | `0x03` POWER+camera | 20 s | 5.60 / 178.6 | off | off | n/a | n/a | n/a | n/a | DMA2D compose active: copy `112`, fallback `0`, error `0`; LTDC errors `0`. |
  | `0x05` POWER+PCMD | 15 s | off | 145.80 / 6.9 | off | n/a | n/a | n/a | n/a | PCMD healthy: masks `0xf`, SAI/DMA errors `0`, quiet `peak/avg=-84/-87 dBFS`. Count includes PCMD reset/config settle; steady counter reports near 187.5 FPS but see tick caveat below. |
  | `0x07` POWER+camera+PCMD | 20 s | 5.60 / 178.6 | 46.70 / 21.4 | off | n/a | n/a | n/a | n/a | Camera still drags PCMD publish/SAI service down without SAI/DMA errors. |
  | `0x0d` POWER+PCMD+acoustic | 30 s | off | 167.10 / 6.0 | 8.83 / 113.2 | 54,923,860 | 91.5 ms / 10.92 FPS | 68.7 ms / 14.57 FPS | coarse 52.3%, fine 44.7%, pre+FFT+GCC 2.8% | This is the clean SRP number after internal RAM + phase-step optimization. |
  | `0x1f` full path | 30 s | 8.73 / 114.5 | 72.17 / 13.9 | 2.73 / 365.9 | 91,381,282 | 152.3 ms / 6.57 FPS | 114.2 ms / 8.75 FPS | coarse 51.7%, fine 44.9%, pre+FFT+GCC 3.3% | Full path still has camera/overlay/PCMD contention; DMA2D copy `262`, fallback `0`, error `0`, overlay draw `231`. |

  Current optimized logs:
  `_debug_logs/perfram_current_mask_3_20260707_102727.gdb.log`,
  `_debug_logs/perfram_current_mask_5_20260707_102829.gdb.log`,
  `_debug_logs/perfram_current_mask_7_20260707_102914.gdb.log`,
  `_debug_logs/perfram_current_mask_d_20260707_103003.gdb.log`, and
  `_debug_logs/perfram_current_mask_1f_20260707_103112.gdb.log`.
- 2026-07-07 measurement caveat found during optimized validation:
  `published_fps_x10`, `acoustic_fps_x10`, and `srp_ms_x100` are not reliable
  in this RAM `PerfRam` run because the HAL tick-derived values appear off by
  about 10x. Use DWT `perf.*_cycles` and host-duration count deltas for
  optimization decisions until the tick scale is fixed or proven hardware-only.
- Current performance conclusion after the first optimization:
  1. SRP improved from the previous clean acoustic-only `~220.6M`/`367.7 ms`
     class to `54.9M cycles`/`91.5 ms @600 MHz` (`68.7 ms @800 MHz`). That is
     now close to 10 FPS at 600 MHz and about 14.6 FPS at 800 MHz, but still
     short of 20 FPS for Balanced/160 pairs.
  2. The SRP bottleneck remains almost entirely search accumulation:
     `coarse + fine` is about `97%` of total cycles. Further gains should focus
     on MVE-friendly dot products, reducing fine candidates, and memory/LUT
     placement, not PCMD or FFT.
  3. Full path is still much slower (`91.4M cycles`, `152.3 ms @600 MHz`), and
     camera scenes continue to reduce PCMD publish rate despite zero SAI/DMA
     errors. Next display/system work should investigate thread priorities,
     interrupt latency, EXTRAM/AXI pressure, and overlay/camera scheduling.
- 2026-07-07 second SRP optimization comparison:
  kept only quality-neutral changes in `app_acoustic_srp.c`. The coarse phase
  setup now derives `sin_start/cos_start` from the already-computed bin phase
  delta, so each pair uses one `arm_sin_cos_f32()` instead of two when phase
  steps are built. The accumulation loop is also pointer-based and unrolled by
  four bins to reduce index arithmetic and improve the next MVE conversion path.
  No pair count, grid count, bin range, or confidence logic was changed.

  | Variant | Scene | SRP cycles | ms @600 MHz | ms @800 MHz | Delta vs previous optimized baseline | Notes |
  | --- | --- | ---: | ---: | ---: | ---: | --- |
  | Previous optimized baseline | `0x0d` POWER+PCMD+acoustic | 54,923,860 | 91.5 | 68.7 | baseline | `.SRP_FAST` + phase-step cache. |
  | Retained phase-start/unroll | `0x0d` POWER+PCMD+acoustic | 53,478,704 | 89.1 | 66.8 | -2.6% | PCMD healthy, processed `300/30s`, failed `0`; log `_debug_logs/perfram_current_mask_d_20260707_105241.gdb.log`. |
  | Fine phase cache trial | `0x0d` POWER+PCMD+acoustic | 52,088,276 | 86.8 | 65.1 | -5.2% | Helped the clean path slightly, but was not kept. |
  | Previous optimized baseline | `0x1f` full path | 91,381,282 | 152.3 | 114.2 | baseline | Camera+PCMD+acoustic+overlay. |
  | Retained phase-start/unroll | `0x1f` full path | 79,802,123 | 133.0 | 99.8 | -12.7% | Camera `~8.7 FPS`, PCMD no SAI/DMA errors, acoustic failed `0`; log `_debug_logs/perfram_current_mask_1f_20260707_105343.gdb.log`. |
  | Fine phase cache trial | `0x1f` full path | 81,115,737 | 135.2 | 101.4 | -11.2% | Worse than retained code in the real full path, so reverted. |

  Decision: do not keep the fine phase cache yet. It adds hot workspace pressure
  and only helps the acoustic-only scene, while the product path gets slower.
  The retained code gives a smaller clean-path gain but a better full-path gain,
  so it is the safer baseline for the next round.
- 2026-07-07 frequency-compressed standard-mode implementation:
  frequency selection is now a runtime-configurable control. `AppAcousticImagingConfig_t`
  carries an explicit `active_bins[]` list and `active_bin_count`, so SRP no
  longer assumes every profile uses a continuous `active_bin_start..end` range.
  Product modes are separate from low-level profiles: `FAST`, `STANDARD`, and
  `HIGH_QUALITY` are selected with `AppAcousticService_SetMode()`, and each mode
  can be switched at runtime. A mode switch resets the bin policy to
  `PROFILE_DEFAULT` so `HIGH_QUALITY` always returns to full B40 unless a test
  override is applied afterward.

  Mode defaults:
  - `FAST`: fast profile, 96 pairs, effective-bin `STANDARD_B12`.
  - `STANDARD`: balanced profile, 160 pairs, effective-bin `STANDARD_B16`.
  - `HIGH_QUALITY`: quality profile, 240 pairs, continuous `QUALITY_B40`.
  - `STANDARD_B24` remains an explicit conservative field-test override.

  `AppAcousticService_SetBinPolicy()` accepts
  `PROFILE_DEFAULT/B12/B16/B24/B40` for controlled experiments, and
  `AppAcousticServiceSnapshot_t` reports requested/active mode, requested/active
  policy, and active bin count for GDB/UI diagnostics.

  The selected standard-mode bins are not arbitrary. They were generated by
  `tools/acoustic_imaging/select_effective_bins.py --random-trials 8`, which
  scores each bin by synthetic localization error, high-SNR agreement, mean PHAT
  peak quality, and long-baseline alias weighting, then applies a low/mid/high
  band-balanced quota. Current firmware tables:
  - `STANDARD_B12`: bins `7,8,9,10,12,14,16,17,26,27,40,41`
    (`1312.5..7687.5 Hz`, sparse).
  - `STANDARD_B16`: bins `6,7,8,9,10,12,14,16,17,20,23,26,27,38,40,41`
    (`1125.0..7687.5 Hz`, sparse), current standard default.
  - `STANDARD_B24`: bins `3,4,5,6,7,8,9,10,12,14,16,17,19,20,23,24,26,27,36,38,39,40,41,42`
    (`562.5..7875.0 Hz`, sparse).
  - `QUALITY_B40`: continuous bins `3..42` (`562.5..7875.0 Hz`).

  Firmware SRP behavior:
  - `QUALITY_B40` remains the continuous 40-bin reference path and still uses
    the CMSIS contiguous GCC path plus the existing unrolled accumulation.
  - `B12/B16/B24` use sparse selected-bin GCC gather and gap-aware steering
    phase recurrence, so only selected FFT bins are processed.
  - NPU heatmap remains disabled for compressed-bin profiles and is accepted
    only for continuous B40 semantics.

  PC tooling:
  - `tools/acoustic_imaging/acoustic_imaging_model.py` now supports fixed bin
    policies and mirrors the firmware coarse 9x9 + NMS top-k + fine-search path.
  - `tools/acoustic_imaging/evaluate_bin_masks.py` compares `QUALITY_B40`,
    `STANDARD_B24`, `STANDARD_B16`, `STANDARD_B12`, and `FAST_B12`, with optional
    single-bin ablation.
  - Quick simulation command:
    `python tools\acoustic_imaging\evaluate_bin_masks.py --random-trials 12`.

  Quick simulation result, 36 scenarios:

  | Candidate | Pairs | Bins | Work vs Quality | P90 | P90 SNR>=10dB | Notes |
  | --- | ---: | ---: | ---: | ---: | ---: | --- |
  | `QUALITY_B40` | 240 | 40 | 100.0% | 2.6 deg | 2.5 deg | Regression reference. |
  | `STANDARD_B24` | 160 | 24 | 40.0% | 2.7 deg | 2.5 deg | Conservative fallback. |
  | `STANDARD_B16` | 160 | 16 | 26.7% | 2.6 deg | 2.5 deg | Default standard-mode candidate. |
  | `STANDARD_B12` | 160 | 12 | 20.0% | 2.8 deg | 3.2 deg | Speed-pressure candidate. |
  | `FAST_B12` | 96 | 12 | 12.0% | 2.6 deg | 2.9 deg | Auto-degrade candidate. |

  Validation completed after this implementation, including the product-mode
  split and explicit bin-policy override: Python unit tests passed,
  `select_effective_bins.py --random-trials 8` reproduced the firmware bin
  tables, `evaluate_bin_masks.py --random-trials 12` passed,
  `evaluate_srp_profiles.py --random-trials 12` passed, and both
  `tools\build_n647_app.ps1 -Configuration Debug` and `-Configuration Release`
  passed. The selector and fixed-mask simulations prove the PC tool path works,
  but they are not a final frequency-selection conclusion because the real board
  still needs noise, aliasing, and full-path DWT validation. Next required
  measurement is N6 DWT cycle comparison for mode defaults plus forced
  `B12/B16/B24/B40`, first acoustic-only (`0x0d`) and then full path (`0x1f`).
- Next engineering step after validation: normalize the whole firmware memory
  allocation model instead of adding ad-hoc sections. Start from the actual
  N647 memory map and linker support, then define named tiers for executable hot
  code, DSP hot data/LUTs, DMA-visible noncacheable buffers, cacheable internal
  SRAM, external frame/media buffers, and experimental/offline buffers. Current
  project linkers only expose generic internal `RAM` at `0x34000000` and
  `.EXTRAM`; do not invent `.ITCM`/`.DTCM`/extra SRAM banks until the N6 memory
  map, MPU/cache policy, and debugger load path are verified.
- 2026-07-07 board validation for fixed-bin standard mode:
  used the RAM debug launch path with Release-like `-Os` optimization, `DEBUG`
  kept only for the RAM/debug boot path, I/D cache enabled
  (`SCB->CCR=0x00030201`), and `SystemCoreClock=600 MHz`. This is the current
  reliable remote validation path because the true external-Flash Release image
  still stops in BootROM with the present boot strap.

  Acoustic-only/product-mode comparison with PCMD running (`g_app_bringup_control_mask=0x0d`):

  | Mode / policy | Pairs | Bins | Total cycles | Time @600MHz | Est. @800MHz | Main cost |
  | --- | ---: | ---: | ---: | ---: | ---: | --- |
  | `STANDARD/B12` forced | 160 | 12 | 41.9M | 69.8 ms | 52.3 ms | coarse/fine search |
  | `STANDARD/B16` forced | 160 | 16 | 48.3M | 80.5 ms | 60.3 ms | coarse/fine search |
  | `STANDARD/B24` forced | 160 | 24 | 64.6M | 107.7 ms | 80.8 ms | coarse/fine search |
  | `HIGH_QUALITY/B40` forced | 240 | 40 | 74.9M | 124.9 ms | 93.6 ms | coarse/fine search |

  Full product path (`g_app_bringup_control_mask=0x1f`) with camera, PCMD,
  acoustic service, and camera overlay active:

  | Mode / policy | Pairs | Bins | Total cycles | Time @600MHz | Est. @800MHz | Notes |
  | --- | ---: | ---: | ---: | ---: | ---: | --- |
  | `STANDARD/B16` product default | 160 | 16 | 83.0M | 138.4 ms | 103.8 ms | About 9.6 FPS at 800 MHz before more optimization. |

  Full-path stage cycles for `STANDARD/B16`: preprocess 0.49M, FFT 0.43M,
  GCC 1.72M, coarse 42.8M, fine 37.5M, output 0.04M. The bottleneck is still
  SRP coarse/fine search, and full path adds significant memory/display-system
  contention compared with acoustic-only (`48.3M -> 83.0M cycles`).
  Symbol-level halt inspection confirmed the same root cause: the final
  full-path sample stopped in `App_AcousticSrp_AdvancePhase()` at
  `app_acoustic_srp.c:550`, inside the sparse selected-bin phase recurrence.
  Earlier transient halts can land in `HAL_DMA2D_PollForTransfer()` or XSPI
  init depending on when GDB interrupts, but the measured acoustic frame cost is
  dominated by SRP search, not by PCMD, FFT/GCC, TouchGFX, or camera ISR time.
  Treat display/camera as a secondary bandwidth/EXTRAM/DMA2D contention source:
  it inflates `STANDARD/B16` from about `48.3M` cycles acoustic-only to
  `83.0M` cycles in the full product path.

  Hardware chain status during full path was healthy enough for algorithm
  validation: PCMD present/config/status masks were all `0xf`, PCMD raw audio
  valid, SAI A/B DMA error counts were zero, and the selected standard bins were
  exactly `6,7,8,9,10,12,14,16,17,20,23,26,27,38,40,41`. Camera/display also
  ran: IMX219 chip id `0x0219`, 411 camera/display swaps in the sample window,
  LTDC layer1/layer2 enabled, overlay update/draw counters increasing, DMA2D
  copy count increasing, and DMA2D fallback/error zero.

  Release build and flash were also verified on the same date:
  `flash_n647_release.ps1 -BuildBundle -ResetAfter` built with zero errors and
  zero warnings, programmed `_flash_images/n647_boot_bundle.hex`, and CubeProgrammer
  verified the download. After both software reset and H7 relay power-cycle,
  `probe_n647_state.ps1` still reported `PC=0x18003514/0x18003518 [BootROM]`.
  Do not treat this as an application regression; it means the current remote
  boot/debug condition is not entering the external-Flash XIP app. True Release
  visual validation still requires the external-boot strap/path to be active.
- 2026-07-07 product UI refactor checkpoint:
  IMAGE is now a camera-first HUD instead of a debug-style panel page. The
  fixed camera/color-key rectangle remains `192,60,640,480`; TouchGFX draws
  the surrounding top/left/right/bottom HUD, and the acoustic heatmap remains
  on the camera overlay path rather than a normal opaque TouchGFX widget.

  UI pages were reorganized for product use:
  - IMAGE: mode/status/FPS/quality/PCMD/source direction HUD around the camera.
  - MICS: 32-channel dBFS health grid plus PCMD status.
  - PERF: acoustic coarse/fine/GCC/FFT/total cycles plus camera/overlay counters.
  - SET: product switches for mode/bin policy/overlay/diagnostics.
  - MEDIA: icon-first screenshot/record/storage controls.

  TouchGFX assets were regenerated from the final compact asset set under
  `NECCS_N647_App/Appli/TouchGFX/assets/images/`, and `texts.xml` was reduced
  to the Chinese/technical wildcard characters actually needed by the current
  handwritten UI. Current generated product assets are logo plus 17 compact
  24x24 status/mode/navigation icons.

  Important memory rule confirmed: generated TouchGFX bitmap/font/text data is
  read-only product data. Linker scripts now collect `ExtFlashSection`,
  `FontFlashSection`, `FontSearchFlashSection`, and `TextFlashSection` into
  `.touchgfx_resources`.
  - Release: `.touchgfx_resources = 0x701613e0, size 0x14480`, mapped to XIP
    `ROM`. This is the product/resource-placement rule.
  - RAM Debug first tried `.touchgfx_resources = 0x90600000` in `EXTRAM`, but
    GDB loading failed before `main` because external RAM is not initialized yet
    at the load step. RAM Debug now maps `.touchgfx_resources` to internal RAM
    instead; the validated load address was `0x341d0140`, size `0x14480`.
  - Debug configuration is intentionally `-Os + DEBUG + -g3` so the RAM Debug
    image fits and remains release-like enough for UI/performance diagnosis. Do
    not use an `-O0` RAM image as the memory or performance baseline.
  - Internal hot SRP data remains in `.SRP_FAST` (`0xbae20` bytes in the
    current build), and camera/media/PCMD large buffers remain in `.EXTRAM`.

  Validation completed for this checkpoint:
  - `tools\debug\n647_debug_env.ps1 -CheckOnly` passed.
  - Official Debug script passed after the RAM-Debug resource-placement fix:
    `tools\build_n647_app.ps1 -Configuration Debug`, 0 errors, 0 warnings,
    `text=431136, data=52964, bss=4772064`.
  - Official Release script passed:
    `tools\build_n647_app.ps1 -Configuration Release`, 0 errors, 0 warnings,
    `text=427360, data=52964, bss=4771936`.
  - H7 relay status/power-cycle path worked on `COM5`, then RAM Debug passed
    with connect-under-reset and stopped at `main` after loading
    `.touchgfx_resources` at `0x341d0140`.
  - A 20 s full-path RAM Debug sample with `g_app_bringup_control_mask=0x1f`
    showed the UI/camera/display path alive:
    `SystemCoreClock=600000000`, `CCR=0x00030201`, camera frames `111`,
    LTDC swaps `111`, LTDC errors `0`, overlay draws `80`, DMA2D copies `111`,
    DMA2D error `0`.
  - PCMD was present/configured/raw-valid in the same run:
    `present=0xf`, `cfg=0xf`, `valid=1`, `rawvalid=1`, `frames=237`,
    `rawpeak=-14 dBFS`, `rawavg=-66 dBFS`. `statusok=0x3` still needs later
    audio-chain follow-up, but it did not block UI validation.
  - Acoustic Standard/B16 still bottlenecked in SRP:
    `processed=10`, `skipped=50`, `bins=16`, `total=86334337 cycles`,
    `coarse=44294879`, `fine=39429955`, `gcc=243459`, `quality=1`,
    `valid=0` in the quiet remote scene.
  - Framebuffer dump/composite showed the actual product HUD, visible camera
    layer, and acoustic overlay cells:
    `_debug_logs/n647_ui_composite_latest.png`.

  If a future RAM Debug load fails before `main`, do not move RAM-Debug
  `.touchgfx_resources` back to `EXTRAM`; the proven rule is Release resources
  in XIP ROM, RAM-Debug resources in internal RAM.
