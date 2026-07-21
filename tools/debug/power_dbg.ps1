# One-shot BQ25730 / power-rail state dump over the live GDB server.
# Usage: .\tools\debug\power_dbg.ps1   (board must be running the Debug build
# with OpenOCD up - run .\tools\debug\n647.ps1 status first if unsure)
$ErrorActionPreference = "Stop"

$gdb = Get-ChildItem "C:\ST" -Recurse -Filter "arm-none-eabi-gdb.exe" -ErrorAction SilentlyContinue |
    Select-Object -First 1 -ExpandProperty FullName
if (-not $gdb) { throw "arm-none-eabi-gdb.exe not found under C:\ST" }

$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$elf = Join-Path $repo "NECCS_N647_App\STM32CubeIDE\Appli\Debug\NECCS_N647_App_Appli.elf"

$cmd = Join-Path $env:TEMP "power_dbg.gdb"
@"
set pagination off
target extended-remote localhost:3333
monitor halt
printf "BQ init=%d probe=%d i2c=%d pins_rd=%d\n", g_app_bq25730_init_status, g_app_bq25730_probe_status, g_app_bq25730_last_i2c_status, g_app_bq25730_pin_read_status
printf "BQ mfg=0x%02x dev=0x%02x (expect 0x40/0xD5)\n", g_app_bq25730_manufacturer_id, g_app_bq25730_device_id
printf "BQ chg_status=0x%04x prochot=0x%04x pins=0x%02x refresh=%u\n", g_app_bq25730_charger_status, g_app_bq25730_prochot_status, g_app_bq25730_pin_state, g_app_bq25730_refresh_count
printf "PWR state=%u flags=0x%03x vbat=%umV vsys=%umV soc=%u%% ibat=%dmA\n", g_app_power_state, g_app_power_flags, g_app_power_battery_mv, g_app_power_system_mv, g_app_power_battery_percent, g_app_power_battery_current_ma
printf "PWR cmpin=%umV remain_mahx1k=%d\n", g_app_power_cmpin_mv, g_app_power_remaining_mah_x1000
printf "POWER_DBG_DONE\n"
monitor resume
detach
quit
"@ | Set-Content $cmd -Encoding ascii

& $gdb --batch -q -x $cmd $elf 2>&1 | ForEach-Object { $_.ToString() } |
    Select-String "BQ |PWR |POWER_DBG_DONE|error"
