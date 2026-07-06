[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Port,
    [int]$OffMs = 5000,
    [int]$Baud = 921600,
    [switch]$RunRamDebug,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $here "..\..\..")
$relayScript = Join-Path $here "usb_power_cycle_h7.ps1"

Write-Host "Power-cycling via H7 relay controller on $Port ..."
& $relayScript -Port $Port -Action Cycle -OffMs $OffMs -Baud $Baud

if (-not $RunRamDebug) {
    Write-Host "Relay cycle complete. Re-run with -RunRamDebug to launch the N647 RAM debug loop."
    return
}

Push-Location $repoRoot
try {
    & .\tools\debug\n647_debug_env.ps1 -CheckOnly

    $args = @("-ConnectUnderReset", "-Batch")
    if ($SkipBuild) {
        $args += "-SkipBuild"
    }

    & .\tools\debug\debug_n647_ram.ps1 @args
} finally {
    Pop-Location
}
