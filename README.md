# NECCS_N647 Project Notes

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
