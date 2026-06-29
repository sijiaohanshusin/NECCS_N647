[CmdletBinding()]
param(
    [switch]$NoStartOpenOcd,
    [switch]$LeaveOpenOcd,
    [switch]$ConnectUnderReset,
    [switch]$AsJson,
    [int]$GdbPort = 3333,
    [int]$SwdKHz = 4000
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "n647_debug_env.ps1")

function ConvertTo-N647AddressValue {
    param([string]$HexValue)

    if ([string]::IsNullOrWhiteSpace($HexValue)) {
        return $null
    }

    return [Convert]::ToUInt64(($HexValue -replace "^0x", ""), 16)
}

function Get-N647RegisterValue {
    param(
        [string[]]$Output,
        [string]$Register
    )

    foreach ($line in $Output) {
        if ($line -match "^\s*$Register\s+0x([0-9a-fA-F]+)") {
            return "0x$($matches[1])"
        }
    }

    return $null
}

function Get-N647AddressClass {
    param([Nullable[UInt64]]$Address)

    if ($null -eq $Address) {
        return "unknown"
    }

    if ($Address -ge 0x18000000 -and $Address -lt 0x18100000) {
        return "BootROM"
    }

    if ($Address -ge 0x34000000 -and $Address -lt 0x34200000) {
        return "RAM Debug / AXI SRAM"
    }

    if ($Address -ge 0x70000000 -and $Address -lt 0x70100000) {
        return "External Flash FSBL region"
    }

    if ($Address -ge 0x70100000 -and $Address -lt 0x70200000) {
        return "External Flash XIP App region"
    }

    if ($Address -ge 0x90000000 -and $Address -lt 0x92000000) {
        return "External RAM"
    }

    return "other"
}

$envInfo = Get-N647DebugEnv
if ($envInfo.Problems.Count -gt 0) {
    Write-N647DebugEnv -EnvInfo $envInfo
    throw "N647 debug environment is incomplete."
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
    $gdbCmdPath = Join-Path $envInfo.LogRoot "gdb_probe_n647_$stamp.cmd"
    $gdbLogPath = Join-Path $envInfo.LogRoot "gdb_probe_n647_$stamp.log"

    $commands = @(
        "set confirm off",
        "set pagination off",
        "target extended-remote localhost:$GdbPort",
        "monitor halt",
        "info registers pc sp lr xpsr",
        "x/8wx `$pc",
        "x/8wx `$sp",
        "detach",
        "quit"
    )

    Set-Content -LiteralPath $gdbCmdPath -Value $commands -Encoding ASCII

    $oldErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        $output = & $envInfo.GdbExe --batch -q -x $gdbCmdPath 2>&1 | ForEach-Object { "$_" }
        $gdbExitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $oldErrorActionPreference
    }
    $output | Set-Content -LiteralPath $gdbLogPath -Encoding ASCII

    if ($gdbExitCode -ne 0) {
        throw "GDB probe failed with exit code $gdbExitCode. See $gdbLogPath"
    }

    $pc = Get-N647RegisterValue -Output $output -Register "pc"
    $sp = Get-N647RegisterValue -Output $output -Register "sp"
    $lr = Get-N647RegisterValue -Output $output -Register "lr"
    $xpsr = Get-N647RegisterValue -Output $output -Register "xpsr"

    $pcValue = ConvertTo-N647AddressValue -HexValue $pc
    $spValue = ConvertTo-N647AddressValue -HexValue $sp

    $result = [pscustomobject]@{
        PC = $pc
        PCClass = Get-N647AddressClass -Address $pcValue
        SP = $sp
        SPClass = Get-N647AddressClass -Address $spValue
        LR = $lr
        XPSR = $xpsr
        GdbLogPath = [System.IO.Path]::GetFullPath($gdbLogPath)
        GdbCommandPath = [System.IO.Path]::GetFullPath($gdbCmdPath)
    }

    if ($AsJson) {
        $result | ConvertTo-Json -Depth 3
    } else {
        Write-Host "N647 target state"
        Write-Host "PC   : $($result.PC) [$($result.PCClass)]"
        Write-Host "SP   : $($result.SP) [$($result.SPClass)]"
        Write-Host "LR   : $($result.LR)"
        Write-Host "XPSR : $($result.XPSR)"
        Write-Host "Log  : $($result.GdbLogPath)"
    }
} finally {
    if ($startedOpenOcd -and -not $LeaveOpenOcd -and $null -ne $openOcdSession.ProcessId) {
        Stop-Process -Id $openOcdSession.ProcessId -ErrorAction SilentlyContinue
    }
}
