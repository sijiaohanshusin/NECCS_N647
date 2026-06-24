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
$coreSourceDir = Join-Path $repoRoot "NECCS_N647_App\Appli\Core\Src"
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

function Write-TextFileNoBom {
    param(
        [string]$Path,
        [string]$Text
    )

    $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $utf8NoBom)
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

function Repair-GeneratedCoreSourceList {
    param([string]$ConfigDir)

    $subdirMk = Join-Path $ConfigDir "Application\User\Core\subdir.mk"
    $objectsList = Join-Path $ConfigDir "objects.list"

    if (-not (Test-Path -LiteralPath $subdirMk -PathType Leaf) -or
        -not (Test-Path -LiteralPath $objectsList -PathType Leaf)) {
        return
    }

    $subdirText = Get-Content -LiteralPath $subdirMk -Raw
    $objectsText = Get-Content -LiteralPath $objectsList -Raw
    $repairBlockPattern = "(?ms)\r?\n?# Added by tools/build_n647_app\.ps1 when CubeIDE omits local user sources\.\r?\nC_SRCS \+= \\\r?\n[^\r\n]+\r?\n\r?\nOBJS \+= \\\r?\n[^\r\n]+\r?\n\r?\nC_DEPS \+= \\\r?\n[^\r\n]+(?:\r?\n\r?\nApplication/User/Core/[^\r\n]+\.o: [^\r\n]+\r?\n\t[^\r\n]+)?"
    $subdirText = [regex]::Replace($subdirText, $repairBlockPattern, "")

    $mainCompileLine = $null
    if ($subdirText -match "(?m)^Application/User/Core/main\.o:.*\r?\n(\tarm-none-eabi-gcc .*)$") {
        $mainCompileLine = $Matches[1]
    } else {
        throw "Cannot locate main.c compile rule in generated makefile: $subdirMk"
    }

    $allSubdirText = $subdirText
    foreach ($otherSubdirMk in Get-ChildItem -LiteralPath $ConfigDir -Recurse -Filter "subdir.mk" -File) {
        if ($otherSubdirMk.FullName -ne $subdirMk) {
            $allSubdirText += "`n" + (Get-Content -LiteralPath $otherSubdirMk.FullName -Raw)
        }
    }

    $newBlocks = New-Object System.Collections.Generic.List[string]
    $objectLines = New-Object System.Collections.Generic.List[string]

    foreach ($source in Get-ChildItem -LiteralPath $coreSourceDir -Filter "*.c" -File) {
        $baseName = [System.IO.Path]::GetFileNameWithoutExtension($source.Name)
        $sourceForMake = $source.FullName -replace "\\", "/"
        $objectForMake = "./Application/User/Core/$baseName.o"
        $dependencyForMake = "./Application/User/Core/$baseName.d"
        $targetForMake = "Application/User/Core/$baseName.o"
        $sourceAlreadyInUserCore = $subdirText -match [regex]::Escape($source.Name)
        $sourceAlreadyGeneratedElsewhere = $allSubdirText -match [regex]::Escape($source.Name)

        if (-not $sourceAlreadyInUserCore -and -not $sourceAlreadyGeneratedElsewhere) {
            $newBlock = @"

# Added by tools/build_n647_app.ps1 when CubeIDE omits local user sources.
C_SRCS += \
$sourceForMake

OBJS += \
$objectForMake

C_DEPS += \
$dependencyForMake

$targetForMake`: $sourceForMake Application/User/Core/subdir.mk
$mainCompileLine
"@
            $newBlocks.Add($newBlock) | Out-Null
            $objectLines.Add("`"$objectForMake`"") | Out-Null

            foreach ($suffix in @(".o", ".d", ".su", ".cyclo")) {
                Remove-Item -LiteralPath (Join-Path $ConfigDir "Application\User\Core\$baseName$suffix") -ErrorAction SilentlyContinue
            }
        }

        if ($sourceAlreadyInUserCore -and $objectsText -match [regex]::Escape($objectForMake)) {
            $objectLines.Add("`"$objectForMake`"") | Out-Null
        }
    }

    if ($newBlocks.Count -gt 0 -or $subdirText -ne (Get-Content -LiteralPath $subdirMk -Raw)) {
        $subdirText = $subdirText.TrimEnd() + "`r`n" + ($newBlocks -join "`r`n") + "`r`n"
        Write-TextFileNoBom -Path $subdirMk -Text $subdirText
        if ($newBlocks.Count -gt 0) {
            Write-Host ("Repaired generated Core source list for {0}: {1} file(s)." -f (Split-Path -Leaf $ConfigDir), $newBlocks.Count)
        }
    }

    $cleanObjectLines = New-Object System.Collections.Generic.List[string]
    foreach ($line in (Get-Content -LiteralPath $objectsList)) {
        if ($line -notmatch '^\s*"\./Application/User/Core/.+\.o"\s*$') {
            $cleanObjectLines.Add($line) | Out-Null
            continue
        }

        if ($line -match '^\s*"\./Application/User/Core/([^/]+)\.o"\s*$') {
            $baseName = $Matches[1]
            if ($subdirText -match [regex]::Escape("$baseName.o")) {
                $cleanObjectLines.Add($line) | Out-Null
            }
        }
    }

    foreach ($line in $objectLines) {
        if (-not ($cleanObjectLines -contains $line)) {
            $cleanObjectLines.Add($line) | Out-Null
        }
    }

    $newObjectsText = ($cleanObjectLines -join "`r`n") + "`r`n"
    if ($newObjectsText -ne $objectsText) {
        Write-TextFileNoBom -Path $objectsList -Text $newObjectsText
        Write-Host ("Repaired objects.list for {0}." -f (Split-Path -Leaf $ConfigDir))
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

        Repair-GeneratedCoreSourceList -ConfigDir $configDir

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
