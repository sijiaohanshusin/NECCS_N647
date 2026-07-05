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
- Runtime breakpoint at `AppPcmdCapture_ProcessHalf()` did not hit before the
  manual stop. GDB interruption showed the target in
  `I2C_IsErrorOccurred()` on `hi2c2`, waiting for I2C STOP/error handling.
  Treat shared I2C2 contention/long PCMD transactions as the current first
  runtime suspect before deeper SAI/audio debugging.
- Follow-up code now exposes `g_app_i2c2_snapshot` and shortens PCMD3180 I2C
  retries/timeouts so failed PCMD config should no longer hold I2C2 long enough
  to freeze touch/UI. The next board run should check `max_hold_ms`,
  `lock_fail_count`, `recover_request_count`, PCMD present/config masks, and
  SAI half/full counters.
