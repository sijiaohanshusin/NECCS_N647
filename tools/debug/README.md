# N647 Debug Tools

This directory provides a scriptable STM32N647 debug loop that does not depend
on CubeIDE launch state.

## Tool Check

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\n647_debug_env.ps1 -CheckOnly
```

Expected tools:

- STMicroelectronics OpenOCD from STM32CubeIDE.
- ST OpenOCD `st_scripts` with `target/stm32n6x.cfg`.
- `arm-none-eabi-gdb` from STM32CubeIDE.
- STM32CubeProgrammer CLI.
- Official `MX25UM25645G_ATK-CNN647B_ExtMemLoader.stldr`.

## Target Probe

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\probe_n647_state.ps1
```

The script starts OpenOCD on `3333/4444/6666`, reads `PC/SP/LR/xPSR`, and
classifies the PC region:

- `0x1800....`: BootROM.
- `0x3400....`: RAM Debug / AXI SRAM.
- `0x7010....`: External Flash XIP App.

Use recovery attach only when normal attach cannot halt the target:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\probe_n647_state.ps1 -ConnectUnderReset
```

## RAM Debug

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\debug_n647_ram.ps1 -SkipBuild -Batch
```

This starts OpenOCD, connects GDB, loads the Debug ELF into RAM, sets `SP/PC`,
and stops at `main`. A valid RAM debug stop should report `PC=0x3400....`.

Omit `-SkipBuild` to build Debug first.

## Release Flash

Validate the release flash command without writing:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\flash_n647_release.ps1 -DryRun
```

Write and verify the boot bundle with the official external loader:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\debug\flash_n647_release.ps1 -BuildBundle -ResetAfter
```

Release flashing uses CubeProgrammer CLI, not GDB `load`.

## Logs

All generated logs are written to `_debug_logs/`, which is intentionally ignored
by git.
