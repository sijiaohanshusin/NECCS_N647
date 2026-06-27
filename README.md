# NECCS_N647 Project Notes

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
