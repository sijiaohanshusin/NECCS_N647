# n647.ps1 - unified board operation entry point.
#
# THE one command any agent should use for board work. Wraps the fragile
# multi-step OpenOCD/GDB/relay sequences into verified one-shot actions.
#
#   .\tools\debug\n647.ps1 devboot          # BOOT1=high + power cycle -> debug boot (SWD/flashing)
#   .\tools\debug\n647.ps1 xipboot          # BOOT1=low  + power cycle -> Release cold boot from flash
#   .\tools\debug\n647.ps1 build            # build Debug config (kills stale CubeIDE javas first)
#   .\tools\debug\n647.ps1 build -Config Release
#   .\tools\debug\n647.ps1 flash-debug      # devboot + build + GDB load + run (the standard dev loop)
#   .\tools\debug\n647.ps1 flash-debug -SkipBuild
#   .\tools\debug\n647.ps1 flash-release    # devboot + bundle rebuild + CubeProgrammer flash + verify
#   .\tools\debug\n647.ps1 status           # one-shot health dump (liveness, PCMD, camera, NPU, touch, media)
#   .\tools\debug\n647.ps1 screenshot       # capture the LCD into _debug_logs\screen_<ts>.png
#   .\tools\debug\n647.ps1 openocd          # (re)start the OpenOCD server only
#
# Conventions this script guarantees:
# - OpenOCD is started with the keepalive config (survives GDB detach).
# - Tool paths are resolved by globbing the newest CubeIDE install.
# - GDB reads use SYMBOL NAMES against the current ELF (addresses drift
#   between builds; never hardcode them).
# - H7 relay/BOOT1 controller is on the CH340 COM port (default COM3,
#   921600 baud, commands: status|on|off|cycle|boot1 low|high|hiz|xipboot|devboot).

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet("devboot", "xipboot", "build", "flash-debug", "flash-release",
                 "status", "screenshot", "openocd", "relay-status", "uitour")]
    [string]$Action,

    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",

    [switch]$SkipBuild,
    [string]$RelayPort = "COM3",
    [int]$OffMs = 3000
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$LogDir = Join-Path $RepoRoot "_debug_logs"
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

# ---------------------------------------------------------------- tool paths
function Resolve-CubeIdeTool {
    param([string]$PluginPattern, [string]$RelPath)
    $ide = Get-ChildItem "C:\ST" -Directory -Filter "STM32CubeIDE_*" |
        Sort-Object Name -Descending | Select-Object -First 1
    if (-not $ide) { throw "STM32CubeIDE not found under C:\ST" }
    $plugin = Get-ChildItem (Join-Path $ide.FullName "STM32CubeIDE\plugins") -Directory -Filter $PluginPattern |
        Sort-Object Name -Descending | Select-Object -First 1
    if (-not $plugin) { throw "plugin $PluginPattern not found" }
    return Join-Path $plugin.FullName $RelPath
}

$script:OpenOcd = Resolve-CubeIdeTool "com.st.stm32cube.ide.mcu.externaltools.openocd.win32_*" "tools\bin\openocd.exe"
$script:OcdScripts = Resolve-CubeIdeTool "com.st.stm32cube.ide.mcu.debug.openocd_*" "resources\openocd\st_scripts"
$script:Gdb = Resolve-CubeIdeTool "com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*" "tools\bin\arm-none-eabi-gdb.exe"
$script:DebugElf = Join-Path $RepoRoot "NECCS_N647_App\STM32CubeIDE\Appli\Debug\NECCS_N647_App_Appli.elf"

# ---------------------------------------------------------------- helpers
function Send-RelayCommand {
    param([string]$Command, [int]$WaitSeconds = 12)
    $port = New-Object System.IO.Ports.SerialPort($RelayPort, 921600, 'None', 8, 'One')
    $port.ReadTimeout = 1500; $port.WriteTimeout = 1500
    $port.DtrEnable = $false; $port.RtsEnable = $false
    $port.Open()
    try {
        $port.DiscardInBuffer()
        $port.Write("$Command`r`n")
        $deadline = (Get-Date).AddSeconds($WaitSeconds)
        $resp = ""
        while ((Get-Date) -lt $deadline) {
            while ($port.BytesToRead -gt 0) { $resp += $port.ReadExisting() }
            if ($resp -match "state=USB_ON") { break }
            Start-Sleep -Milliseconds 200
        }
        return ($resp -replace "`r`n", " | ")
    } finally {
        $port.Close()
    }
}

function Start-OpenOcdServer {
    Get-Process -Name openocd -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 1
    Start-Process -FilePath $script:OpenOcd `
        -ArgumentList '-s', "`"$script:OcdScripts`"", '-f', 'tools/debug/n647_openocd_ram.cfg' `
        -WorkingDirectory $RepoRoot `
        -RedirectStandardOutput (Join-Path $LogDir "openocd_n647.out") `
        -RedirectStandardError (Join-Path $LogDir "openocd_n647.err") | Out-Null
    Start-Sleep -Seconds 5
    $tail = Get-Content (Join-Path $LogDir "openocd_n647.err") -Tail 2
    if (-not ($tail -match "Listening on port 3333")) {
        throw "OpenOCD did not come up: $tail (board may need 'devboot' first)"
    }
    Write-Host "OpenOCD up (gdb :3333, telnet :4444)"
}

function Test-OpenOcdAlive {
    return $null -ne (Get-Process -Name openocd -ErrorAction SilentlyContinue)
}

function Invoke-Build {
    Get-Process -Name java, javaw -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
    & (Join-Path $RepoRoot "tools\build_n647_app.ps1") -Config $Config
    if ($LASTEXITCODE -ne 0) { throw "build failed" }
}

function Invoke-GdbScript {
    param([string]$ScriptBody)
    $tmp = Join-Path $LogDir ("gdb_n647_{0}.cmd" -f (Get-Date -Format "HHmmss"))
    @"
set confirm off
set pagination off
set mem inaccessible-by-default off
file "$($script:DebugElf.Replace('\','/'))"
target extended-remote localhost:3333
$ScriptBody
quit
"@ | Out-File -FilePath $tmp -Encoding ascii
    # GDB chatters on stderr; that must not abort the wrapper.
    $prev = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    $out = & $script:Gdb --batch -x $tmp 2>&1 | ForEach-Object { $_.ToString() }
    $ErrorActionPreference = $prev
    return $out
}

# ---------------------------------------------------------------- actions
switch ($Action) {
    "relay-status" {
        Send-RelayCommand "status" 3
    }

    "devboot" {
        Get-Process -Name openocd -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
        Write-Host (Send-RelayCommand "devboot $OffMs")
        Write-Host "board rebooted into DEBUG boot mode (BOOT1=1, BootROM waits, SWD available)"
    }

    "xipboot" {
        Get-Process -Name openocd -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
        Write-Host (Send-RelayCommand "xipboot $OffMs")
        Write-Host "board cold-booted into RELEASE (XIP) mode from external flash"
    }

    "openocd" {
        Start-OpenOcdServer
    }

    "build" {
        Invoke-Build
    }

    "flash-debug" {
        Write-Host (Send-RelayCommand "devboot $OffMs")
        if (-not $SkipBuild) { Invoke-Build }
        Start-OpenOcdServer
        $out = Invoke-GdbScript @"
monitor reset halt
load
set `$sp = (unsigned int)&_estack
set `$pc = (unsigned int)Reset_Handler
tbreak main
continue
printf "tag=HIT_MAIN\n"
monitor resume
printf "tag=RUNNING\n"
"@
        if (-not ($out -match "tag=RUNNING")) { throw "load failed: $($out | Select-Object -Last 5)" }
        Write-Host "Debug build loaded and running (RAM @0x3400....)"
    }

    "flash-release" {
        Write-Host (Send-RelayCommand "devboot $OffMs")
        Get-Process -Name openocd -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
        if ($SkipBuild) {
            & (Join-Path $PSScriptRoot "flash_n647_release.ps1")
        } else {
            & (Join-Path $PSScriptRoot "flash_n647_release.ps1") -BuildBundle
        }
        Write-Host "release bundle flashed+verified. Run '.\tools\debug\n647.ps1 xipboot' to cold boot it."
    }

    "status" {
        if (-not (Test-OpenOcdAlive)) { Start-OpenOcdServer }
        $out = Invoke-GdbScript @"
monitor halt
printf "tag=STATUS\n"
printf "uptime_ms=%u\n", uwTick
printf "pcmd: present=0x%x cfg_ok=0x%x started=%u frames=%u wdog=%u raw_valid=%u active_slots=%u\n", 'app_pcmd_capture.c'::s_snapshot.device_present_mask, 'app_pcmd_capture.c'::s_snapshot.device_config_ok_mask, 'app_pcmd_capture.c'::s_snapshot.started, 'app_pcmd_capture.c'::s_snapshot.published_frames, 'app_pcmd_capture.c'::s_snapshot.watchdog_restart_count, 'app_pcmd_capture.c'::s_snapshot.raw_audio_valid, 'app_pcmd_capture.c'::s_snapshot.raw_active_slot_count
printf "camera: frames=%u\n", g_app_camera_frame_count
printf "npu: init=%u count=%u us=%u status=%u\n", 'app_npu.c'::s_npu.initialized, 'app_npu.c'::s_npu.inference_count, 'app_npu.c'::s_npu.last_us, 'app_npu.c'::s_npu.last_status
printf "touch: ready=%u down=%u samples=%u wire_err=%u\n", 'app_touch.c'::g_touch.ready, 'app_touch.c'::g_touch.down, 'app_touch.c'::g_touch.sample_count, 'app_touch.c'::g_touch.wire_error_count
printf "media: mounted=%u shots=%u videos=%u err=%d\n", 'app_media.c'::s_media_mounted, 'app_media.c'::s_status.screenshots, 'app_media.c'::s_status.videos, 'app_media.c'::s_status.last_error
monitor resume
printf "tag=OK\n"
"@
        $out | Select-String "tag=|uptime|pcmd:|camera:|npu:|touch:|media:"
        # second uptime read proves the core is actually running
        Start-Sleep -Seconds 2
        $out2 = Invoke-GdbScript @"
monitor halt
printf "uptime2_ms=%u\n", uwTick
monitor resume
"@
        $out2 | Select-String "uptime2"
        Write-Host "(uptime2 > uptime means the firmware is alive, not hung)"
    }

    "screenshot" {
        if (-not (Test-OpenOcdAlive)) { Start-OpenOcdServer }
        $png = Join-Path $LogDir ("screen_{0}.png" -f (Get-Date -Format "MMdd_HHmmss"))
        & (Join-Path $PSScriptRoot "capture_n647_screen.ps1") -NoStartOpenOcd -OutPng $png
        Write-Host "saved: $png"
    }

    "uitour" {
        # Walk every UI page via the Model's remote-screen hook and capture
        # each one. THE verification step after any UI change.
        if (-not (Test-OpenOcdAlive)) { Start-OpenOcdServer }
        $pages = @(
            @{ id = 0; name = "image" },
            @{ id = 1; name = "mics" },
            @{ id = 2; name = "perf" },
            @{ id = 3; name = "settings" },
            @{ id = 4; name = "media" }
        )
        $stamp = Get-Date -Format "MMdd_HHmmss"
        foreach ($page in $pages) {
            [void](Invoke-GdbScript "set g_app_ui_request_screen = $($page.id)`nmonitor resume")
            Start-Sleep -Seconds 2
            $png = Join-Path $LogDir ("uitour_{0}_{1}.png" -f $stamp, $page.name)
            & (Join-Path $PSScriptRoot "capture_n647_screen.ps1") -NoStartOpenOcd -OutPng $png
            Write-Host "page $($page.name): $png"
        }
        [void](Invoke-GdbScript "set g_app_ui_request_screen = 0`nmonitor resume")
        Write-Host "uitour complete - review every PNG above with an image reader"
    }
}
