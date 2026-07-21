# Gimbal / laser bring-up control over the live GDB server (Debug build).
# Requires OpenOCD running (any n647.ps1 action leaves it up).
#
# Usage:
#   .\tools\debug\gimbal_test.ps1 state              # read everything, change nothing
#   .\tools\debug\gimbal_test.ps1 laser              # toggle laser (PC10, auto-raises 7.4V rail)
#   .\tools\debug\gimbal_test.ps1 center             # enable servos + slew to (0,0)
#   .\tools\debug\gimbal_test.ps1 point -Theta 20 -Phi -10   # enable + slew to angle (deg)
#   .\tools\debug\gimbal_test.ps1 enable             # toggle servo enable (PC7 rail + PWM)
#   .\tools\debug\gimbal_test.ps1 relay              # toggle PD0 relay output
#   .\tools\debug\gimbal_test.ps1 charge -Ma 512     # request charge current (mA, 0=stop, clamp 2048)
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet("state", "laser", "center", "point", "enable", "relay", "charge")]
    [string]$Action,
    [int]$Theta = 0,
    [int]$Phi = 0,
    [int]$Ma = 0
)
# gdb prints its halt banner to stderr; keep that from becoming a
# terminating NativeCommandError under strict preference.
$ErrorActionPreference = "Continue"

$gdb = Get-ChildItem "C:\ST" -Recurse -Filter "arm-none-eabi-gdb.exe" -ErrorAction SilentlyContinue |
    Select-Object -First 1 -ExpandProperty FullName
if (-not $gdb) { throw "arm-none-eabi-gdb.exe not found under C:\ST" }

$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$elf = Join-Path $repo "NECCS_N647_App\STM32CubeIDE\Appli\Debug\NECCS_N647_App_Appli.elf"

switch ($Action) {
    "laser"  { $poke = "set var g_app_gimbal_test_request = 2" }
    "center" { $poke = "set var g_app_gimbal_test_theta = 0`nset var g_app_gimbal_test_phi = 0`nset var g_app_gimbal_test_request = 1" }
    "point"  { $poke = "set var g_app_gimbal_test_theta = $Theta`nset var g_app_gimbal_test_phi = $Phi`nset var g_app_gimbal_test_request = 1" }
    "enable" { $poke = "set var g_app_gimbal_test_request = 4" }
    "relay"  { $poke = "set var g_app_gimbal_test_request = 3" }
    "charge" { $poke = "set var 'app_power.c'::g_app_power_charge_request_ma = $Ma" }
    default  { $poke = "" }
}

$cmd = Join-Path $env:TEMP "gimbal_test.gdb"
@"
set pagination off
target extended-remote localhost:3333
monitor halt
$poke
monitor resume
detach
quit
"@ | Set-Content $cmd -Encoding ascii

& $gdb --batch -q -x $cmd $elf 2>&1 | Out-Null
if ($Action -ne "state") {
    # The hook is consumed by AppGimbal_Poll / AppPower_Poll on their next
    # tick; give them a beat before reading the result back.
    Start-Sleep -Milliseconds 1500
}

$cmd2 = Join-Path $env:TEMP "gimbal_state.gdb"
@"
set pagination off
target extended-remote localhost:3333
monitor halt
printf "GIMBAL init=%u enabled=%u rail_on=%u laser=%u relay=%u\n", 'app_gimbal.c'::s_initialized, 'app_gimbal.c'::s_enabled, 'app_gimbal.c'::s_power_on, 'app_gimbal.c'::s_laser_on, 'app_gimbal.c'::s_relay_on
printf "PINS PC7(rail)=%u PC10(laser)=%u PD0(relay)=%u\n", ((*(unsigned int *)0x56020814 >> 7) & 1), ((*(unsigned int *)0x56020814 >> 10) & 1), ((*(unsigned int *)0x56020C14 >> 0) & 1)
printf "CHG request=%umA dirty=%u\n", 'app_power.c'::s_charge_request_ma, 'app_power.c'::s_charge_dirty
printf "GIMBAL_TEST_DONE\n"
monitor resume
detach
quit
"@ | Set-Content $cmd2 -Encoding ascii

& $gdb --batch -q -x $cmd2 $elf 2>&1 | ForEach-Object { $_.ToString() } |
    Select-String "GIMBAL |PINS |CHG |GIMBAL_TEST_DONE|error"
