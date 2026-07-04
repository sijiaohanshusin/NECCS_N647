[CmdletBinding()]
param(
    [switch]$SkipBuild,
    [switch]$NoStartOpenOcd,
    [switch]$LeaveOpenOcd,
    [switch]$ConnectUnderReset,
    [switch]$NoLoad,
    [switch]$Batch,
    [int]$GdbPort = 3333,
    [int]$SwdKHz = 4000,
    [string]$ElfPath
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "n647_debug_env.ps1")

function ConvertTo-N647GdbPath {
    param([string]$Path)
    return ([System.IO.Path]::GetFullPath($Path) -replace "\\", "/")
}

$envInfo = Get-N647DebugEnv
if ($envInfo.Problems.Count -gt 0) {
    Write-N647DebugEnv -EnvInfo $envInfo
    throw "N647 debug environment is incomplete."
}

if (-not $SkipBuild) {
    $buildScript = Join-Path $envInfo.RepoRoot "tools\build_n647_app.ps1"
    & $buildScript -Configuration Debug
    if ($LASTEXITCODE -ne 0) {
        throw "Debug build failed with exit code $LASTEXITCODE."
    }
}

if ([string]::IsNullOrWhiteSpace($ElfPath)) {
    $ElfPath = $envInfo.DebugElf
}

if (-not (Test-Path -LiteralPath $ElfPath -PathType Leaf)) {
    throw "Debug ELF not found: $ElfPath"
}

if (-not (Test-Path -LiteralPath $envInfo.LogRoot -PathType Container)) {
    New-Item -ItemType Directory -Path $envInfo.LogRoot | Out-Null
}

$openOcdSession = $null
$startedOpenOcd = $false

try {
    if (-not $NoStartOpenOcd) {
        $startOpenOcd = Join-Path $PSScriptRoot "start_n647_openocd.ps1"
        $startArgs = @{
            GdbPort = $GdbPort
            SwdKHz = $SwdKHz
            Wait = $true
        }
        if ($ConnectUnderReset) {
            $startArgs.ConnectUnderReset = $true
        }
        $openOcdSession = & $startOpenOcd @startArgs
        $startedOpenOcd = $true
    }

    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $gdbCmdPath = Join-Path $envInfo.LogRoot "gdb_n647_ram_$stamp.cmd"
    $gdbLogPath = Join-Path $envInfo.LogRoot "gdb_n647_ram_$stamp.log"
    $gdbElf = ConvertTo-N647GdbPath -Path $ElfPath

    $commands = New-Object System.Collections.Generic.List[string]
    $commands.Add("set confirm off")
    $commands.Add("set pagination off")
    $commands.Add("set mem inaccessible-by-default off")
    $commands.Add("file `"$gdbElf`"")
    $commands.Add("target extended-remote localhost:$GdbPort")
    $commands.Add("monitor reset halt")
    if (-not $NoLoad) {
        $commands.Add("load")
    }
    $commands.Add("set `$sp = (unsigned int)&_estack")
    $commands.Add("set `$pc = (unsigned int)Reset_Handler")
    $commands.Add("tbreak main")
    $commands.Add("continue")
    $commands.Add("info registers pc sp lr xpsr")
    if ($Batch) {
        $commands.Add("quit")
    }

    Set-Content -LiteralPath $gdbCmdPath -Value $commands -Encoding ASCII

    $gdbArgs = @("-q", "-x", $gdbCmdPath)
    if ($Batch) {
        $gdbArgs = @("--batch") + $gdbArgs
    }

    Write-Host "Starting GDB RAM debug:"
    Write-Host "  gdb : $($envInfo.GdbExe)"
    Write-Host "  elf : $ElfPath"
    Write-Host "  cmd : $gdbCmdPath"
    Write-Host "  log : $gdbLogPath"

    $oldErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        & $envInfo.GdbExe @gdbArgs 2>&1 | ForEach-Object { "$_" } | Tee-Object -FilePath $gdbLogPath
        $gdbExitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $oldErrorActionPreference
    }
    if ($gdbExitCode -ne 0) {
        throw "GDB failed with exit code $gdbExitCode. See $gdbLogPath"
    }
} finally {
    if ($startedOpenOcd -and -not $LeaveOpenOcd -and $null -ne $openOcdSession.ProcessId) {
        Stop-Process -Id $openOcdSession.ProcessId -ErrorAction SilentlyContinue
    }
}
