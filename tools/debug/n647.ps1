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
#   .\tools\debug\n647.ps1 screenshot -Freeze # pause camera DMA first: keeps the
#                                             # acoustic overlay in the dump (else
#                                             # DCMIPP overwrites it post-halt)
#   .\tools\debug\n647.ps1 openocd          # (re)start the OpenOCD server only
#
# Conventions this script guarantees:
# - OpenOCD is started with the keepalive config (survives GDB detach).
# - Tool paths are resolved by globbing the newest CubeIDE install.
# - GDB reads use SYMBOL NAMES against the current ELF (addresses drift
#   between builds; never hardcode them).
# - H7 relay/BOOT1 controller is a CH340 (VID_1A86&PID_7523) COM port,
#   921600 baud, text commands: status|on|off|cycle|boot1 low|high|hiz|
#   xipboot|devboot. Port is auto-detected by USB PID (COM number drifts
#   when the lab USB tree changes; was COM3, then COM8).
# - USB 5V relay (in the VBUS wire of the N647's OTG1 Type-C link) is a
#   CH341 (VID_1A86&PID_5523) COM port speaking the LCUS "A0" protocol at
#   9600: ch1 ON = A0 01 01 A2, OFF = A0 01 00 A1, query = A0 01 05 A6
#   (feedback variants 03/02). BOTH supplies must be cut for a true power
#   cycle, so devboot/xipboot drop USB 5V first and leave it OFF (USB
#   data does not need VBUS: the board has no VBUS sense and powers from
#   the H7-switched rail; usb5v-on re-enables charging/back-feed on
#   purpose).
#   .\tools\debug\n647.ps1 usb5v-on|usb5v-off|usb5v-status

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet("devboot", "xipboot", "build", "flash-debug", "flash-release",
                 "status", "screenshot", "openocd", "relay-status", "uitour",
                 "usb5v-on", "usb5v-off", "usb5v-status")]
    [string]$Action,

    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",

    [switch]$SkipBuild,
    [switch]$Freeze,
    [string]$RelayPort = "",
    [string]$Usb5vPort = "",
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
function Resolve-RelayComPort {
    # H7 controller = CH340 (PID_7523), USB 5V relay = CH341 (PID_5523).
    # COM numbers drift with the lab USB tree; match by PID instead.
    param([string]$PidToken)
    $dev = Get-PnpDevice -Class Ports -PresentOnly -ErrorAction SilentlyContinue |
        Where-Object { $_.InstanceId -match "VID_1A86&$PidToken" } |
        Select-Object -First 1
    if ($dev -and ($dev.FriendlyName -match "\((COM\d+)\)")) { return $Matches[1] }
    return $null
}

function Get-H7Port {
    if ($RelayPort) { return $RelayPort }
    $port = Resolve-RelayComPort "PID_7523"
    if (-not $port) { throw "H7 relay controller (CH340, VID_1A86&PID_7523) not found" }
    return $port
}

function Get-Usb5vPort {
    if ($Usb5vPort) { return $Usb5vPort }
    return Resolve-RelayComPort "PID_5523"   # absent = no USB relay wired, callers tolerate
}

function Send-RelayCommand {
    param([string]$Command, [int]$WaitSeconds = 12)
    $port = New-Object System.IO.Ports.SerialPort((Get-H7Port), 921600, 'None', 8, 'One')
    $port.ReadTimeout = 1500; $port.WriteTimeout = 1500
    $port.DtrEnable = $false; $port.RtsEnable = $false
    $port.Open()
    try {
        $port.DiscardInBuffer()
        $port.Write("$Command`r`n")
        $deadline = (Get-Date).AddSeconds($WaitSeconds)
        $resp = ""
        while ((Get-Date) -lt $deadline) {
            # A power cycle can glitch the CH340's USB hub: the handle then
            # throws "port closed" even though the command was already sent.
            try {
                while ($port.BytesToRead -gt 0) { $resp += $port.ReadExisting() }
            } catch {
                $resp += " (com-glitch: $($_.Exception.Message))"
                break
            }
            if ($resp -match "state=USB_ON") { break }
            Start-Sleep -Milliseconds 200
        }
        return ($resp -replace "`r`n", " | ")
    } finally {
        if ($port.IsOpen) { $port.Close() }
    }
}

# USB 5V relay (LCUS "A0" protocol). op: 0x00 off, 0x01 on (no reply),
# 0x02 off + feedback, 0x03 on + feedback, 0x05 query. Returns reply hex.
function Send-Usb5vOp {
    param([byte]$Op)
    $comName = Get-Usb5vPort
    if (-not $comName) { return $null }
    $port = New-Object System.IO.Ports.SerialPort($comName, 9600, 'None', 8, 'One')
    $port.ReadTimeout = 1200; $port.WriteTimeout = 1200
    $port.Open()
    try {
        $port.DiscardInBuffer()
        $sum = [byte]((0xA0 + 0x01 + $Op) % 256)
        $port.Write([byte[]](0xA0, 0x01, $Op, $sum), 0, 4)
        Start-Sleep -Milliseconds 500
        $n = $port.BytesToRead
        if ($n -gt 0) {
            $buf = New-Object byte[] $n
            $port.Read($buf, 0, $n) | Out-Null
            return (($buf | ForEach-Object { $_.ToString('X2') }) -join ' ')
        }
        return "(no reply)"
    } finally {
        $port.Close()
    }
}

# Cut USB-supplied 5V ahead of an H7 power cycle (both rails must drop for
# the BootROM strap latch to reset). Leaves it OFF: USB data needs no VBUS
# on this board, and back-feed would defeat every future power cycle.
function Disable-Usb5vForPowerCycle {
    $reply = Send-Usb5vOp 0x02
    if ($null -eq $reply) {
        Write-Host "usb5v: relay not present, assuming no USB back-feed"
    } else {
        Write-Host "usb5v: cut (reply $reply)"
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
        Disable-Usb5vForPowerCycle
        Write-Host (Send-RelayCommand "devboot $OffMs")
        Write-Host "board rebooted into DEBUG boot mode (BOOT1=1, BootROM waits, SWD available)"
    }

    "xipboot" {
        Get-Process -Name openocd -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
        Disable-Usb5vForPowerCycle
        Write-Host (Send-RelayCommand "xipboot $OffMs")
        Write-Host "board cold-booted into RELEASE (XIP) mode from external flash"
    }

    "usb5v-on" {
        $reply = Send-Usb5vOp 0x03
        if ($null -eq $reply) { throw "USB 5V relay (CH341, VID_1A86&PID_5523) not found" }
        Write-Host "usb5v ON (reply $reply) - NOTE: H7 power cycles no longer reset the board until usb5v-off"
    }

    "usb5v-off" {
        $reply = Send-Usb5vOp 0x02
        if ($null -eq $reply) { throw "USB 5V relay (CH341, VID_1A86&PID_5523) not found" }
        Write-Host "usb5v OFF (reply $reply)"
    }

    "usb5v-status" {
        $reply = Send-Usb5vOp 0x05
        if ($null -eq $reply) { throw "USB 5V relay (CH341, VID_1A86&PID_5523) not found" }
        # A0 01 01 A2 = energized (5V passing), A0 01 00 A1 = released (cut)
        Write-Host "usb5v relay query reply: $reply  (01=on/passing, 00=off/cut)"
    }

    "openocd" {
        Start-OpenOcdServer
    }

    "build" {
        Invoke-Build
    }

    "flash-debug" {
        Disable-Usb5vForPowerCycle
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
        Disable-Usb5vForPowerCycle
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
printf "pcmd: present=0x%x cfg_ok=0x%x started=%u frames=%u wdog=%u raw_valid=%u active_slots=%u silent=0x%x shift=0x%x heals=%u\n", 'app_pcmd_capture.c'::s_snapshot.device_present_mask, 'app_pcmd_capture.c'::s_snapshot.device_config_ok_mask, 'app_pcmd_capture.c'::s_snapshot.started, 'app_pcmd_capture.c'::s_snapshot.published_frames, 'app_pcmd_capture.c'::s_snapshot.watchdog_restart_count, 'app_pcmd_capture.c'::s_snapshot.raw_audio_valid, 'app_pcmd_capture.c'::s_snapshot.raw_active_slot_count, 'app_pcmd_capture.c'::s_snapshot.silent_chip_mask, 'app_pcmd_capture.c'::s_snapshot.shift_chip_mask, 'app_pcmd_capture.c'::s_snapshot.silent_restart_count
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
        if ($Freeze) {
            # Park the camera pipe so the overlay-carrying frame survives the
            # halt. The swap worker applies the freeze right after its next
            # overlay draw + flip (<=1 camera frame), so the dumped frame
            # matches the screen exactly.
            [void](Invoke-GdbScript "set g_app_camera_freeze_request = 1`nmonitor resume")
            Start-Sleep -Milliseconds 800
        }
        try {
            & (Join-Path $PSScriptRoot "capture_n647_screen.ps1") -NoStartOpenOcd -OutPng $png
        }
        finally {
            if ($Freeze) {
                [void](Invoke-GdbScript "set g_app_camera_freeze_request = 0`nmonitor resume")
            }
        }
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
