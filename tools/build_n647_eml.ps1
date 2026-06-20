[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "All")]
    [string]$Configuration = "All",

    [string]$CubeIdeRoot = $env:STM32CUBEIDE_ROOT
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$projectDir = Join-Path $repoRoot "NECCS_N647_ExtMemLoader\NECCS_N647_ExtMemLoader\ExtMemLoader"
$workspaceDir = Join-Path $repoRoot "_cubeide_ws_eml_build"
$projectName = "NECCS_N647_ExtMemLoader_ExtMemLoader"
$languageSettings = Join-Path $projectDir ".settings\language.settings.xml"

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
    throw "EML CubeIDE project not found: $projectDir"
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

        $outputDir = Join-Path $projectDir $config
        $elf = Join-Path $outputDir "$projectName.elf"
        $loader = Join-Path $outputDir "$projectName.stldr"

        if (-not (Test-Path -LiteralPath $elf -PathType Leaf)) {
            throw "$config build completed without generating: $elf"
        }

        if (-not (Test-Path -LiteralPath $loader -PathType Leaf)) {
            throw "$config build completed without generating: $loader"
        }
    }
} finally {
    # CubeIDE rewrites this scanner-cache hash on every machine and dirties Git.
    if ($null -ne $languageSettingsBackup) {
        [System.IO.File]::WriteAllBytes($languageSettings, $languageSettingsBackup)
    }
}

Write-Host "N647 EML build completed successfully."
