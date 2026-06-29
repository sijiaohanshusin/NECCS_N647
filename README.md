# NECCS_N647 Project Notes

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
