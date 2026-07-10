# One-command Debug loader for the N647 board.
#
#   .\tools\debug\load_n647_debug.ps1                 # load + run (board already powered)
#   .\tools\debug\load_n647_debug.ps1 -PowerCycle     # relay power cycle first (needs the COM port free)
#   .\tools\debug\load_n647_debug.ps1 -Build          # rebuild Debug before loading
#
# Loads the RAM-debug ELF over SWD, starts it, then detaches so the app keeps
# running standalone ("放飞"). Requires the BOOT strap in debug position.
#
# TIP: if you keep the relay's serial console open yourself, -PowerCycle
# cannot grab the port - just power cycle from your console first, then run
# this script with no switches.

[CmdletBinding()]
param(
    [switch]$Build,
    [switch]$PowerCycle,
    [string]$RelayPort = "COM8",
    [int]$SwdKHz = 1000
)

$ErrorActionPreference = "Stop"
$repo = "D:\Project\NECCS\Program\NECCS_N647"
Set-Location $repo

$gdb = "C:\ST\STM32CubeIDE_2.1.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740\tools\bin\arm-none-eabi-gdb.exe"
$elf = Join-Path $repo "NECCS_N647_App\STM32CubeIDE\Appli\Debug\NECCS_N647_App_Appli.elf"

# 0) stale debug processes would hold the ST-LINK / workspace
Get-Process -Name openocd, arm-none-eabi-gdb -ErrorAction SilentlyContinue |
    Stop-Process -Force -ErrorAction SilentlyContinue

if ($Build) {
    Write-Host "[1/4] Building Debug ..." -ForegroundColor Cyan
    Get-Process -Name java, javaw -ErrorAction SilentlyContinue |
        Stop-Process -Force -ErrorAction SilentlyContinue
    & powershell -ExecutionPolicy Bypass -File (Join-Path $repo "tools\build_n647_app.ps1") -Config Debug
    if ($LASTEXITCODE -ne 0) { throw "Debug build failed." }
} else {
    Write-Host "[1/4] Skipping build (use -Build to rebuild)" -ForegroundColor DarkGray
}

if (-not (Test-Path $elf)) { throw "Debug ELF not found: $elf" }
Write-Host ("      ELF: {0}  ({1})" -f $elf, (Get-Item $elf).LastWriteTime)

if ($PowerCycle) {
    Write-Host "[2/4] Power cycling via $RelayPort ..." -ForegroundColor Cyan
    $oldEapPc = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    & powershell -ExecutionPolicy Bypass -File (Join-Path $repo "tools\usb_power_relay\h7_relay_controller\usb_power_cycle_h7.ps1") -Port $RelayPort -Action Cycle -OffMs 4000 2>&1 | Out-Null
    $pcOk = ($LASTEXITCODE -eq 0)
    $ErrorActionPreference = $oldEapPc
    if (-not $pcOk) {
        Write-Host "      Relay port busy - power cycle the board manually, then press Enter." -ForegroundColor Yellow
        [void](Read-Host)
    }
    Start-Sleep -Seconds 3
} else {
    Write-Host "[2/4] Skipping power cycle (use -PowerCycle if the link is stuck)" -ForegroundColor DarkGray
}

Write-Host "[3/4] Starting OpenOCD ..." -ForegroundColor Cyan
$session = $null
for ($attempt = 0; $attempt -lt 3; $attempt++) {
    try {
        $session = & (Join-Path $repo "tools\debug\start_n647_openocd.ps1") -Wait -SwdKHz $SwdKHz
        break
    } catch {
        Write-Host "      OpenOCD start failed (attempt $($attempt + 1)/3), retrying ..." -ForegroundColor Yellow
        Get-Process -Name openocd -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 2
    }
}
if ($null -eq $session) { throw "OpenOCD would not start after 3 attempts. Power cycle the board and retry." }

Write-Host "[4/4] Loading and starting the app ..." -ForegroundColor Cyan
$gdbScript = Join-Path $env:TEMP "n647_load_run.cmd"
@"
set confirm off
set pagination off
set mem inaccessible-by-default off
file "$($elf -replace '\\', '/')"
target extended-remote localhost:3333
monitor reset halt
load
set `$sp = (unsigned int)&_estack
set `$pc = (unsigned int)Reset_Handler
tbreak main
continue
printf "tag=HIT_MAIN\n"
monitor resume
printf "tag=RUNNING\n"
quit
"@ | Set-Content -LiteralPath $gdbScript -Encoding ASCII

$oldEap = $ErrorActionPreference
$ErrorActionPreference = "Continue"
$output = & $gdb --batch -q -x $gdbScript 2>&1 | ForEach-Object { "$_" }
$gdbExit = $LASTEXITCODE
$ErrorActionPreference = $oldEap

# Leave the app running standalone: kill the OpenOCD server (GDB already
# resumed the core; dropping the server does not stop it).
Stop-Process -Id $session.ProcessId -ErrorAction SilentlyContinue

if (($gdbExit -eq 0) -and ($output -match "tag=RUNNING")) {
    Write-Host ""
    Write-Host "OK - Debug firmware loaded and running (detached)." -ForegroundColor Green
} else {
    $output | Select-Object -Last 8
    Write-Host ""
    Write-Host "FAILED - see output above. Try again with -PowerCycle." -ForegroundColor Red
    exit 1
}
