[CmdletBinding()]
param(
    [string]$BundleHex,
    [string]$ExternalLoader,
    [ValidateSet("UR", "HOTPLUG", "NORMAL", "HWRSTPULSE")]
    [string]$Mode = "UR",
    [int]$Ap = 1,
    [int]$FrequencyKHz = 4000,
    [switch]$BuildBundle,
    [switch]$NoVerify,
    [switch]$ResetAfter,
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "n647_debug_env.ps1")

$envInfo = Get-N647DebugEnv -ExternalLoader $ExternalLoader
if ($envInfo.Problems.Count -gt 0) {
    Write-N647DebugEnv -EnvInfo $envInfo
    throw "N647 debug environment is incomplete."
}

if ([string]::IsNullOrWhiteSpace($BundleHex)) {
    $BundleHex = $envInfo.ReleaseBundleHex
}

if ($BuildBundle) {
    $rebuild = Join-Path $envInfo.RepoRoot "tools\rebuild_n647_boot_images.ps1"
    & $rebuild
    if ($LASTEXITCODE -ne 0) {
        throw "Boot bundle rebuild failed with exit code $LASTEXITCODE."
    }
}

if (-not (Test-Path -LiteralPath $BundleHex -PathType Leaf)) {
    throw "Release boot bundle not found: $BundleHex. Run tools\rebuild_n647_boot_images.ps1 or pass -BuildBundle."
}

if (-not (Test-Path -LiteralPath $envInfo.ExternalLoader -PathType Leaf)) {
    throw "External loader not found: $($envInfo.ExternalLoader)"
}

$connect = "port=SWD ap=$Ap mode=$Mode reset=HWrst freq=$FrequencyKHz"
$args = @(
    "-c", $connect,
    "-el", $envInfo.ExternalLoader,
    "-w", ([System.IO.Path]::GetFullPath($BundleHex))
)

if (-not $NoVerify) {
    $args += "-v"
}

if ($ResetAfter) {
    $args += "-rst"
}

Write-Host "Flashing N647 release bundle:"
Write-Host "  cli    : $($envInfo.CubeProgrammerCli)"
Write-Host "  connect: $connect"
Write-Host "  loader : $($envInfo.ExternalLoader)"
Write-Host "  bundle : $([System.IO.Path]::GetFullPath($BundleHex))"
Write-Host "  verify : $(-not $NoVerify)"
Write-Host "  dry run: $([bool]$DryRun)"

if ($DryRun) {
    Write-Host "Dry run only. CubeProgrammer command was not executed."
    return
}

& $envInfo.CubeProgrammerCli @args
if ($LASTEXITCODE -ne 0) {
    throw "STM32CubeProgrammer failed with exit code $LASTEXITCODE."
}
