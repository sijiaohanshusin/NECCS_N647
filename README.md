# NECCS_N647 Project Notes

## 2026-07-06 N647 bring-up reset point

- Current checkpoint before the cleanup/rebuild pass:
  `3f084b08 wip: checkpoint n6 pcmd acoustic bringup`.
- Active integration target: final product path, built in layers:
  clock/memory -> display/touch -> I2C -> camera -> PCMD raw capture ->
  audio frame -> acoustic SRP -> camera overlay.
- Do not debug from memory. The current bug ledger, module order, GDB symbols,
  and validation results live in `docs/n647_bringup_status.md`.
- Current firmware exposes `g_app_bringup_snapshot` and `g_app_i2c2_snapshot`
  for GDB-first diagnosis before changing UI or algorithm code.
- DMA nodes and RX buffers for SAI/GPDMA must be inside the MPU-configured
  `.noncacheable` region before DCache is enabled. If `HAL_DMA_ERROR_USE`
  appears on the first SAI callback, check MPU/cache coherency before changing
  PCMD registers.
- PCMD online does not mean MIC audio is valid. As of the latest PCMD-focused
  RAM debug run, main now matches the known-good `7e12d5d` branch: quiet raw DMA
  samples are low-amplitude two's-complement PCM, `raw_audio_valid=1`,
  `latest_frame_valid=1`, and published frames advance. Keep this as the PCMD
  baseline before changing SRP/heatmap code.
- 2026-07-06 after a real target 5V power cycle, the PCMD-to-SRP path reached
  `App_AcousticSrp_ProcessFrame()` and returned `APP_ACOUSTIC_IMAGING_OK` on a
  real `TDM_CAPTURE` frame (`48 kHz`, `32ch`, `256` samples). In quiet bench
  conditions the SRP snapshot is expected to remain low confidence/invalid, not
  show a strong fake heat spot. Multi-frame checks can leave stale GDB/OpenOCD
  sessions and dirty target state; clean them and power-cycle before drawing
  firmware conclusions.
- Follow-up multi-frame RAM Debug confirmed the same path remains healthy after
  several SRP frames: PCMD masks stay `0xf`, raw audio is valid, MIC dBFS is
  quiet (`-78..-90 dBFS` in the remote bench), and SAI/DMA errors remain zero.
  The current blocker has moved to performance: Debug/O0 auto-degrades to Fast
  (`96 pairs`) and still takes about `280 ms/frame`, so Release/optimized
  profiling is required before enabling a 20 FPS product overlay.
- Full product-path RAM Debug now shows camera, PCMD, and SRP running together:
  camera reaches hundreds of LTDC swaps with zero LTDC errors, UI color-key
  holes are correct, PCMD stays valid, and SRP processes real frames. The
  camera display path must be validated with IMX219 test pattern before changing
  LTDC/TouchGFX. Default live gain is deliberately low-noise
  (`exposure=0x0d00`, `analog=0x80`, `digital=0x0100`); dark live frames should
  be treated as lighting/lens/ISP work, not fixed by maxing sensor gain.
- Acoustic overlay bring-up currently uses a forced preview mode on the IMAGE
  page: SRP output is drawn even at low confidence so the camera/overlay path
  can be verified remotely. Draw the overlay on the independent `.EXTRAM`
  compose framebuffer, not directly on the DCMIPP `0x90400000/0x90500000`
  ping-pong buffers; otherwise camera DMA can reuse the displayed buffer and
  tear the heatmap into horizontal fragments. Tighten the confidence gate again
  after live-sound/FPS validation.
- Layered debug can disable heavy modules at the `main()` GDB breakpoint by
  writing `g_app_bringup_control_mask`. Keep the POWER bit set for hardware
  modules in the current state machine: `0x00` UI only, `0x03` POWER+camera,
  `0x05` POWER+PCMD, `0x07` POWER+camera+PCMD, `0x0d` POWER+PCMD+acoustic,
  `0x23` POWER+camera test pattern, default `0x1f` full product path. Do not
  use the old `0x02/0x06/0x22` masks for hardware measurements.
- Camera black-screen diagnosis now has GDB-readable IMX219 register readbacks
  and sampled framebuffer stats. Compare normal `0x03` against test-pattern
  `0x23` before changing LTDC or TouchGFX overlay code again.
- Hardware debug uses the scripted N647 loop first:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\n647_debug_env.ps1 -CheckOnly
powershell -ExecutionPolicy Bypass -File .\tools\debug\debug_n647_ram.ps1 -Batch
```

- If AP0 is visible but AP1/core access is stuck, use the H7 relay helper in
  `tools/usb_power_relay/h7_relay_controller/` before changing firmware. The
  verified COM5 path is:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\usb_power_relay\h7_relay_controller\usb_power_cycle_h7.ps1 -Port COM5 -Action Status
powershell -ExecutionPolicy Bypass -File .\tools\usb_power_relay\h7_relay_controller\usb_power_cycle_h7.ps1 -Port COM5 -Action Cycle -OffMs 5000
```

- 2026-07-06 relay observation: `OFF` made PB1 high and the relay actuated, but
  Windows still listed the ST-LINK; CubeProgrammer showed the target AP count
  drop to `0`. Treat this wiring as target/target-side power recovery, not a
  guaranteed ST-LINK USB re-enumeration.
- For N647 RAM Debug recovery, connect-under-reset uses a local OpenOCD RAM cfg
  override so GDB attach performs a plain `halt` instead of the stock
  `reset init`, which times out on this board. The verified command is:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\debug_n647_ram.ps1 -ConnectUnderReset -SkipBuild -Batch
```

- Release flashing is allowed only after RAM Debug passes:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\flash_n647_release.ps1 -BuildBundle -ResetAfter
```

## 2026-06-28 N6 SRP-PHAT runtime initial port

- Added `app_acoustic_srp.*`, a hardware-neutral SRP-PHAT runtime for
  `Wide32 @ 48 kHz / BALANCED`: 32 planar F32 channels, `NFFT=256`, active bins
  `3..42`, `160` pairs, 9x9 coarse search, and top3 x 4x4 fine search.
- Added `app_acoustic_synthetic.*` to generate deterministic 32-channel planar
  F32 plane-wave frames, so the algorithm path can be verified before real
  capture is enabled.
- The SRP runtime uses the N6/CMSIS-DSP F32 path first and records DWT cycle
  counters for preprocess, FFT, GCC, coarse search, fine search, output, and
  total runtime. F16 and NPU heatmap backends are reserved as unsupported
  extension points.
- Large SRP work buffers are placed in `.EXTRAM` with 32-byte alignment; only
  the small context lives in normal RAM.
- CubeIDE now links the required CMSIS-DSP F32 source files from the STM32Cube
  repository and defines `ARM_MATH_AUTOVECTORIZE` for the current stable build
  path.
- No capture callback, hardware initialization, ThreadX task, or UI publishing
  was added in this step.

## 2026-06-27 Acoustic imaging route foundation

- Added a hardware-neutral `audio_frame` contract in `app_audio_frame.*` above
  the passive microphone mapper. Algorithm code now has a clean frame shape:
  mode, sample rate, channel count, frame length, planar I16/F32 data, sequence,
  timestamp, and drop/error counters.
- Added `app_acoustic_imaging.*` for the first SRP-PHAT route foundation:
  `Wide32 @ 48 kHz` defaults to `NFFT=256`, active bins `3..42`, 9x9 coarse
  grid, top3 x 4x4 fine-search contract, and `BALANCED=160` pair selection.
  `FAST=96` and `QUALITY=240` are available; `Core16 @ 192 kHz` is defined as a
  phase-2 profile with `NFFT=512` and up to 120 pairs.
- Added PC-side offline tools under `tools/acoustic_imaging/` to generate pair
  sets/TDOA LUTs and run synthetic SRP sanity checks from
  `docs/knowledge/microphone-array/array_32ch_coords.csv`.
- PCMD configuration, initialization, debug tasks, SAI start, and DMA callback
  wiring remain intentionally untouched while hardware is under investigation.

## 2026-06-27 Microphone array abstraction

- Added a passive microphone-array mapping layer in `app_mic_array.*`.
- It maps theoretical Bus A/Bus B TDM slot data into the main-scheme logical
  mic order: `M01..M32` for `Wide32 @ 48 kHz`, and `M01..M16` for
  `Core16 @ 192 kHz`.
- This layer does not start SAI, DMA, I2C, PCMD3180 initialization, or any PCMD
  debug task. It is ready for the future DMA callback/audio pipeline handoff
  once the hardware wiring issue is resolved.
- Verified `tools/build_n647_app.ps1` with both `Release` and `Debug`: 0 errors,
  0 warnings.

## 2026-06-26 OCR and microphone array references

- The full OCRKB generated corpus is still present locally after branch
  switching: `docs/ocrkb/documents/` currently contains 9585 files across
  `H7_Original`, `N647_BaseBoard`, `N657_MainBoard`, and `Shared_References`.
- The generated OCR exports, logs, state, and catalogs are intentionally ignored
  by Git via `.gitignore`; the tracked recovery points are `docs/ocrkb/INDEX.md`,
  `docs/ocrkb/README.md`, `docs/ocrkb/reports/quality_report.md`, and
  `docs/ocrkb/run_ocrkb_all.ps1`.
- The microphone array design that lived on `codex/pcmd3180-screen-freeze` has
  been collected under `docs/knowledge/microphone-array/`.

## 2026-06-26 ThreadX clean main merge

- `main` enables ThreadX Core through `NECCS_N647_App.ioc` and CubeMX-generated APP files.
- The APP bring-up loop now runs as a ThreadX thread in `app_bringup_thread.c`; `main.c` keeps hardware init, HyperRAM self-test, LCD init, and `MX_ThreadX_Init()`.
- PCMD configuration, initialization, application debug tasks, and `app_pcmd_*` code were intentionally not merged.
- ThreadX owns `SysTick`, `SVC`, and `PendSV`; `App_ThreadX_SysTickHook()` keeps HAL tick counting inside the ThreadX SysTick handler.
- Verified on 2026-06-26: `powershell -ExecutionPolicy Bypass -File .\tools\build_n647_app.ps1 -Configuration Release` completes with 0 errors / 0 warnings.

## Build

Run from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build_n647_app.ps1 -Configuration Release
```

## NPU Bring-Up Without MIC Hardware

The first NPU integration step is intentionally independent of the microphone
hardware. `APP_ACOUSTIC_BACKEND_NPU_HEATMAP` is now reserved for a Wide32 /
BALANCED coarse heatmap backend that consumes synthetic or replayed GCC-PHAT
features. It is disabled by default through `APP_ACOUSTIC_NPU_HEATMAP_ENABLE=0`,
so the current F32 CMSIS-DSP SRP path remains the default.

Generate the no-dependency NPU model spec:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\acoustic_imaging\generate_npu_heatmap_model.ps1 -SpecOnly
```

Full ST Edge AI generation additionally requires the optional Python `onnx`
package, then the same script can emit the ONNX model and call `stedgeai`.
