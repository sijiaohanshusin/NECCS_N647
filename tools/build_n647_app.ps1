[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "All")]
    [string]$Configuration = "Release",

    [int]$Jobs = [Environment]::ProcessorCount,

    [string]$CubeIdeRoot = $env:STM32CUBEIDE_ROOT,

    [switch]$NoClean,

    [switch]$CopyToFlashImages
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$projectDir = Join-Path $repoRoot "NECCS_N647_App\STM32CubeIDE\Appli"
$projectName = "NECCS_N647_App_Appli"
$iocFile = Join-Path $repoRoot "NECCS_N647_App\NECCS_N647_App.ioc"
$mspFile = Join-Path $repoRoot "NECCS_N647_App\Appli\Core\Src\stm32n6xx_hal_msp.c"
$artifactRows = New-Object System.Collections.Generic.List[object]

function Resolve-CubeIdeRoot {
    param([string]$RequestedRoot)

    if (-not [string]::IsNullOrWhiteSpace($RequestedRoot)) {
        return $RequestedRoot
    }

    $install = Get-ChildItem -LiteralPath "C:\ST" -Directory -Filter "STM32CubeIDE_*" -ErrorAction SilentlyContinue |
        Sort-Object Name -Descending |
        Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName "STM32CubeIDE\plugins") } |
        Select-Object -First 1

    if ($null -eq $install) {
        throw "STM32CubeIDE not found. Set STM32CUBEIDE_ROOT or pass -CubeIdeRoot."
    }

    return Join-Path $install.FullName "STM32CubeIDE"
}

function Find-CubeIdeToolDirectory {
    param(
        [string]$IdeRoot,
        [string]$PluginPattern
    )

    $plugin = Get-ChildItem -LiteralPath (Join-Path $IdeRoot "plugins") -Directory -Filter $PluginPattern |
        Sort-Object Name -Descending |
        Select-Object -First 1

    if ($null -eq $plugin) {
        throw "CubeIDE plugin not found: $PluginPattern"
    }

    return Join-Path $plugin.FullName "tools\bin"
}

function Assert-ContainsText {
    param(
        [string]$Path,
        [string]$Text
    )

    if (-not (Select-String -LiteralPath $Path -SimpleMatch $Text -Quiet)) {
        throw "Required project constraint is missing from ${Path}: ${Text}"
    }
}

function Invoke-Make {
    param(
        [string]$ConfigDir,
        [string]$Make,
        [string]$Target
    )

    Push-Location $ConfigDir
    try {
        & $Make "-j$Jobs" $Target
        if ($LASTEXITCODE -ne 0) {
            throw "make $Target failed in $ConfigDir (exit code $LASTEXITCODE)."
        }
    } finally {
        Pop-Location
    }
}

function Add-ArtifactRow {
    param(
        [string]$Config,
        [string]$Path
    )

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "$Config build completed without generating: $Path"
    }

    $artifactRows.Add([pscustomobject]@{
        Configuration = $Config
        Artifact = $Path
        Size = (Get-Item -LiteralPath $Path).Length
    }) | Out-Null
}

if ($Jobs -lt 1) {
    throw "-Jobs must be greater than 0."
}

if (-not (Test-Path -LiteralPath $projectDir -PathType Container)) {
    throw "APP CubeIDE project not found: $projectDir"
}

Assert-ContainsText -Path $iocFile -Text "PWR.PowerDomain2=PWR_VDDIO_RANGE_1V8"
Assert-ContainsText -Path $iocFile -Text "PWR.PowerDomain3=PWR_VDDIO_RANGE_1V8"
Assert-ContainsText -Path $mspFile -Text "HAL_PWREx_ConfigVddIORange(PWR_VDDIO2,PWR_VDDIO_RANGE_1V8);"
Assert-ContainsText -Path $mspFile -Text "HAL_PWREx_ConfigVddIORange(PWR_VDDIO3,PWR_VDDIO_RANGE_1V8);"

$CubeIdeRoot = Resolve-CubeIdeRoot -RequestedRoot $CubeIdeRoot
$gnuTools = Find-CubeIdeToolDirectory -IdeRoot $CubeIdeRoot -PluginPattern "com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32*.win32_*"
$makeTools = Find-CubeIdeToolDirectory -IdeRoot $CubeIdeRoot -PluginPattern "com.st.stm32cube.ide.mcu.externaltools.make.win32_*"
$make = Join-Path $makeTools "make.exe"

foreach ($tool in @($make, (Join-Path $gnuTools "arm-none-eabi-gcc.exe"), (Join-Path $gnuTools "arm-none-eabi-objcopy.exe"))) {
    if (-not (Test-Path -LiteralPath $tool -PathType Leaf)) {
        throw "Required build tool not found: $tool"
    }
}

$configurations = if ($Configuration -eq "All") {
    @("Debug", "Release")
} else {
    @($Configuration)
}

$oldPath = $env:Path
try {
    $env:Path = "$gnuTools;$makeTools;$env:Path"

    foreach ($config in $configurations) {
        $configDir = Join-Path $projectDir $config
        $makefile = Join-Path $configDir "makefile"
        if (-not (Test-Path -LiteralPath $makefile -PathType Leaf)) {
            throw "$config makefile not found. Generate/build the CubeIDE project once first: $makefile"
        }

        Write-Host "Building $projectName/$config with direct make..."
        if (-not $NoClean) {
            Invoke-Make -ConfigDir $configDir -Make $make -Target "clean"
        }
        Invoke-Make -ConfigDir $configDir -Make $make -Target "all"

        foreach ($artifactName in @("$projectName.elf", "$projectName.bin", "$projectName.map", "$projectName.list")) {
            Add-ArtifactRow -Config $config -Path (Join-Path $configDir $artifactName)
        }
    }
} finally {
    $env:Path = $oldPath
}

$releaseHex = Join-Path $repoRoot "NECCS_N647_App\Binary\appli.hex"
if ($configurations -contains "Release") {
    Add-ArtifactRow -Config "Release" -Path $releaseHex

    $hexHead = Get-Content -LiteralPath $releaseHex -TotalCount 2
    if ($hexHead[0] -ne ":0200000470107A" -or $hexHead[1] -notlike ":10040000*") {
        throw "Application HEX address check failed. Expected vector table at 0x70100400: $releaseHex"
    }

    if ($CopyToFlashImages) {
        $flashImagesDir = Join-Path $repoRoot "_flash_images"
        $flashAppHex = Join-Path $flashImagesDir "appli.hex"
        New-Item -ItemType Directory -Path $flashImagesDir -Force | Out-Null
        Copy-Item -LiteralPath $releaseHex -Destination $flashAppHex -Force
        Add-ArtifactRow -Config "Release" -Path $flashAppHex
    }
}

Write-Host "N647 APP build completed successfully."
Write-Host ""
Write-Host "Generated artifacts:"
foreach ($row in $artifactRows) {
    Write-Host ("[{0}] {1} ({2} bytes)" -f $row.Configuration, $row.Artifact, $row.Size)
}
