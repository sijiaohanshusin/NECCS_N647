[CmdletBinding()]
param(
    [int]$GdbPort = 3333,
    [int]$TelnetPort = 4444,
    [int]$TclPort = 6666,
    [int]$SwdKHz = 4000,
    [switch]$ConnectUnderReset,
    [switch]$Foreground,
    [switch]$Wait,
    [switch]$Force,
    [string]$LogPath
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "n647_debug_env.ps1")

function Test-N647PortAvailable {
    param([int]$Port)

    $listener = Get-NetTCPConnection -LocalPort $Port -State Listen -ErrorAction SilentlyContinue |
        Select-Object -First 1

    return ($null -eq $listener)
}

function Stop-N647DebugPortOwner {
    param([int]$Port)

    $connections = Get-NetTCPConnection -LocalPort $Port -State Listen -ErrorAction SilentlyContinue
    foreach ($connection in $connections) {
        $process = Get-Process -Id $connection.OwningProcess -ErrorAction SilentlyContinue
        if ($null -eq $process) {
            continue
        }

        if ($process.ProcessName -notin @("openocd", "ST-LINK_gdbserver")) {
            throw "Port $Port is owned by $($process.ProcessName)[$($process.Id)]. Refusing to stop it."
        }

        Write-Host "Stopping $($process.ProcessName)[$($process.Id)] on port $Port ..."
        Stop-Process -Id $process.Id -Force
    }
}

function Wait-N647OpenOcdPort {
    param(
        [int]$Port,
        [int]$TimeoutMs,
        [System.Diagnostics.Process]$Process
    )

    $deadline = [DateTime]::UtcNow.AddMilliseconds($TimeoutMs)
    while ([DateTime]::UtcNow -lt $deadline) {
        if ($null -ne $Process -and $Process.HasExited) {
            throw "OpenOCD exited early with code $($Process.ExitCode)."
        }

        if (-not (Test-N647PortAvailable -Port $Port)) {
            return
        }

        Start-Sleep -Milliseconds 200
    }

    throw "Timed out waiting for OpenOCD GDB port $Port."
}

function ConvertTo-N647OpenOcdPath {
    param([string]$Path)

    return ([System.IO.Path]::GetFullPath($Path) -replace "\\", "/")
}

function ConvertTo-N647CommandLine {
    param([string[]]$Arguments)

    $quoted = foreach ($argument in $Arguments) {
        if ($null -eq $argument) {
            '""'
        } elseif ($argument.Length -eq 0) {
            '""'
        } elseif ($argument -match '[\s"]') {
            '"' + ($argument -replace '"', '\"') + '"'
        } else {
            $argument
        }
    }

    return ($quoted -join " ")
}

$envInfo = Get-N647DebugEnv
if ($envInfo.Problems.Count -gt 0) {
    Write-N647DebugEnv -EnvInfo $envInfo
    throw "N647 debug environment is incomplete."
}

foreach ($port in @($GdbPort, $TelnetPort, $TclPort)) {
    if (-not (Test-N647PortAvailable -Port $port)) {
        if ($Force) {
            Stop-N647DebugPortOwner -Port $port
        } else {
            throw "Port $port is already in use. Re-run with -Force only if it is an old OpenOCD/ST-LINK GDB server."
        }
    }
}

if (-not (Test-Path -LiteralPath $envInfo.OpenOcdRamCfg -PathType Leaf)) {
    throw "OpenOCD RAM config not found: $($envInfo.OpenOcdRamCfg)"
}

if (-not (Test-Path -LiteralPath $envInfo.LogRoot -PathType Container)) {
    New-Item -ItemType Directory -Path $envInfo.LogRoot | Out-Null
}

$stamp = Get-Date -Format "yyyyMMdd_HHmmss"
if ([string]::IsNullOrWhiteSpace($LogPath)) {
    $LogPath = Join-Path $envInfo.LogRoot "openocd_n647_ram_$stamp.log"
}

$stdoutPath = Join-Path $envInfo.LogRoot "openocd_stdout_$stamp.log"
$stderrPath = Join-Path $envInfo.LogRoot "openocd_stderr_$stamp.log"

$openOcdArgs = @(
    "-s", (ConvertTo-N647OpenOcdPath -Path $envInfo.OpenOcdScripts),
    "-c", "set GDB_PORT $GdbPort",
    "-c", "set TELNET_PORT $TelnetPort",
    "-c", "set TCL_PORT $TclPort",
    "-c", "set CLOCK_FREQ $SwdKHz",
    "-c", "set CONNECT_UNDER_RESET $([int][bool]$ConnectUnderReset)",
    "-f", (ConvertTo-N647OpenOcdPath -Path $envInfo.OpenOcdRamCfg),
    "-l", (ConvertTo-N647OpenOcdPath -Path $LogPath)
)

Write-Host "Starting OpenOCD:"
Write-Host "  exe     : $($envInfo.OpenOcdExe)"
Write-Host "  scripts : $($envInfo.OpenOcdScripts)"
Write-Host "  cfg     : $($envInfo.OpenOcdRamCfg)"
Write-Host "  gdb     : $GdbPort"
Write-Host "  telnet  : $TelnetPort"
Write-Host "  tcl     : $TclPort"
Write-Host "  swd kHz : $SwdKHz"
Write-Host "  cur     : $([bool]$ConnectUnderReset)"
Write-Host "  log     : $LogPath"

if ($Foreground) {
    & $envInfo.OpenOcdExe @openOcdArgs
    exit $LASTEXITCODE
}

$process = Start-Process `
    -FilePath $envInfo.OpenOcdExe `
    -ArgumentList (ConvertTo-N647CommandLine -Arguments $openOcdArgs) `
    -RedirectStandardOutput $stdoutPath `
    -RedirectStandardError $stderrPath `
    -WindowStyle Hidden `
    -PassThru

if ($Wait) {
    Wait-N647OpenOcdPort -Port $GdbPort -TimeoutMs 7000 -Process $process
}

[pscustomobject]@{
    ProcessId = $process.Id
    GdbPort = $GdbPort
    TelnetPort = $TelnetPort
    TclPort = $TclPort
    LogPath = [System.IO.Path]::GetFullPath($LogPath)
    StdoutPath = [System.IO.Path]::GetFullPath($stdoutPath)
    StderrPath = [System.IO.Path]::GetFullPath($stderrPath)
}
