[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "All")]
    [string]$Configuration = "All",

    [string]$CubeIdeRoot = $env:STM32CUBEIDE_ROOT
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$projectDir = Join-Path $repoRoot "NECCS_N647_App\STM32CubeIDE\Appli"
$workspaceDir = Join-Path $repoRoot "_cubeide_ws_build"
$projectName = "NECCS_N647_App_Appli"
$languageSettings = Join-Path $projectDir ".settings\language.settings.xml"
$iocFile = Join-Path $repoRoot "NECCS_N647_App\NECCS_N647_App.ioc"
$mspFile = Join-Path $repoRoot "NECCS_N647_App\Appli\Core\Src\stm32n6xx_hal_msp.c"

if ([string]::IsNullOrWhiteSpace($CubeIdeRoot)) {
    $install = Get-ChildItem -LiteralPath "C:\ST" -Directory -Filter "STM32CubeIDE_*" -ErrorAction SilentlyContinue |
        Sort-Object Name -Descending |
        Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName "STM32CubeIDE\headless-build.bat") } |
        Select-Object -First 1

    if ($null -ne $install) {
        $CubeIdeRoot = Join-Path $install.FullName "STM32CubeIDE"
    }
}

if ([string]::IsNullOrWhiteSpace($CubeIdeRoot)) {
    throw "STM32CubeIDE not found. Set STM32CUBEIDE_ROOT or pass -CubeIdeRoot."
}

$headlessBuild = Join-Path $CubeIdeRoot "headless-build.bat"

if (-not (Test-Path -LiteralPath $headlessBuild -PathType Leaf)) {
    throw "STM32CubeIDE headless builder not found: $headlessBuild"
}

if (-not (Test-Path -LiteralPath $projectDir -PathType Container)) {
    throw "APP CubeIDE project not found: $projectDir"
}

$boardConstraints = @(
    @($iocFile, "PWR.PowerDomain2=PWR_VDDIO_RANGE_1V8"),
    @($iocFile, "PWR.PowerDomain3=PWR_VDDIO_RANGE_1V8"),
    @($mspFile, "HAL_PWREx_ConfigVddIORange(PWR_VDDIO2,PWR_VDDIO_RANGE_1V8);"),
    @($mspFile, "HAL_PWREx_ConfigVddIORange(PWR_VDDIO3,PWR_VDDIO_RANGE_1V8);")
)

foreach ($constraint in $boardConstraints) {
    if (-not (Select-String -LiteralPath $constraint[0] -SimpleMatch $constraint[1] -Quiet)) {
        throw "ATK-CNN647B VDDIO constraint is missing from $($constraint[0]): $($constraint[1])"
    }
}

$configurations = if ($Configuration -eq "All") {
    @("Debug", "Release")
} else {
    @($Configuration)
}

$languageSettingsBackup = if (Test-Path -LiteralPath $languageSettings -PathType Leaf) {
    [System.IO.File]::ReadAllBytes($languageSettings)
} else {
    $null
}

try {
    foreach ($config in $configurations) {
        Write-Host "Building $projectName/$config ..."

        & $headlessBuild `
            -data $workspaceDir `
            -import $projectDir `
            -cleanBuild "$projectName/$config"

        if ($LASTEXITCODE -ne 0) {
            throw "STM32CubeIDE build failed for $config (exit code $LASTEXITCODE)."
        }
    }
} finally {
    # CubeIDE rewrites this scanner-cache hash on every machine and dirties Git.
    if ($null -ne $languageSettingsBackup) {
        [System.IO.File]::WriteAllBytes($languageSettings, $languageSettingsBackup)
    }
}

$releaseHex = Join-Path $repoRoot "NECCS_N647_App\Binary\appli.hex"
if (($configurations -contains "Release") -and
    (-not (Test-Path -LiteralPath $releaseHex -PathType Leaf))) {
    throw "Release build completed without generating: $releaseHex"
}

Write-Host "N647 APP build completed successfully."
