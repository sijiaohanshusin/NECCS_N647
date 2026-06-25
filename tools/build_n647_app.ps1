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

function Get-ConfigRelativeMakePath {
    param(
        [string]$ConfigDir,
        [string]$Path
    )

    $configFullPath = (Resolve-Path -LiteralPath $ConfigDir).Path
    $pathFullName = (Get-Item -LiteralPath $Path).FullName
    return ($pathFullName.Substring($configFullPath.Length + 1) -replace "\\", "/")
}

function Get-GeneratedSubdirMakefiles {
    param([string]$ConfigDir)

    return @(Get-ChildItem -LiteralPath $ConfigDir -Recurse -Filter "subdir.mk" -File |
        Sort-Object {
            $relativePath = Get-ConfigRelativeMakePath -ConfigDir $ConfigDir -Path $_.FullName
            if ($relativePath -like "Drivers/STM32N6xx_HAL_Driver/*") { "00_$relativePath" }
            elseif ($relativePath -like "Drivers/CMSIS/*") { "01_$relativePath" }
            elseif ($relativePath -like "Drivers/BSP/*") { "02_$relativePath" }
            elseif ($relativePath -like "Application/User/Startup/*") { "03_$relativePath" }
            elseif ($relativePath -like "Application/User/Core/*") { "04_$relativePath" }
            else { "99_$relativePath" }
        })
}

function Get-MakeVariableAppendEntries {
    param(
        [string]$Path,
        [string]$Variable
    )

    $entries = New-Object System.Collections.Generic.List[string]
    $lines = Get-Content -LiteralPath $Path
    $collecting = $false

    foreach ($line in $lines) {
        if ($collecting) {
            if ([string]::IsNullOrWhiteSpace($line)) {
                $collecting = $false
                continue
            }

            $entry = ($line -replace "\\\s*$", "").Trim()
            if (-not [string]::IsNullOrWhiteSpace($entry)) {
                $entries.Add($entry) | Out-Null
            }

            if ($line -notmatch "\\\s*$") {
                $collecting = $false
            }

            continue
        }

        if ($line -match "^\s*$([regex]::Escape($Variable))\s*\+=\s*(.*)$") {
            $rest = $Matches[1].Trim()
            if ($rest -eq "\") {
                $collecting = $true
                continue
            }

            if (-not [string]::IsNullOrWhiteSpace($rest)) {
                foreach ($entry in ($rest -split "\s+")) {
                    $entry = ($entry -replace "\\\s*$", "").Trim()
                    if (-not [string]::IsNullOrWhiteSpace($entry)) {
                        $entries.Add($entry) | Out-Null
                    }
                }
            }

            if ($line -match "\\\s*$") {
                $collecting = $true
            }
        }
    }

    return @($entries)
}

function Get-MainCompileLine {
    param(
        [string]$ConfigDir,
        [string]$CoreSubdirText
    )

    if ($CoreSubdirText -match "(?m)^Application/User/Core/main\.o:.*\r?\n(\tarm-none-eabi-gcc .*)$") {
        return $Matches[1]
    }

    throw "Cannot locate main.c compile rule in generated makefile: $(Join-Path $ConfigDir 'Application\User\Core\subdir.mk')"
}

function Add-MissingLocalBspSubdirMakefiles {
    param(
        [string]$ConfigDir,
        [string]$MainCompileLine
    )

    $bspRoot = Join-Path $repoRoot "NECCS_N647_App\Drivers\BSP"
    if (-not (Test-Path -LiteralPath $bspRoot -PathType Container)) {
        return
    }

    foreach ($sourceDir in Get-ChildItem -LiteralPath $bspRoot -Directory) {
        $sources = @(Get-ChildItem -LiteralPath $sourceDir.FullName -Filter "*.c" -File)
        if ($sources.Count -eq 0) {
            continue
        }

        $relativeDir = "Drivers/BSP/$($sourceDir.Name)"
        $subdirMk = Join-Path $ConfigDir "$relativeDir/subdir.mk"
        if (Test-Path -LiteralPath $subdirMk -PathType Leaf) {
            continue
        }

        New-Item -ItemType Directory -Path (Split-Path -Parent $subdirMk) -Force | Out-Null

        $cSrcLines = New-Object System.Collections.Generic.List[string]
        $objLines = New-Object System.Collections.Generic.List[string]
        $depLines = New-Object System.Collections.Generic.List[string]
        $ruleLines = New-Object System.Collections.Generic.List[string]
        $cleanFiles = New-Object System.Collections.Generic.List[string]

        foreach ($source in $sources) {
            $baseName = [System.IO.Path]::GetFileNameWithoutExtension($source.Name)
            $sourceForMake = $source.FullName -replace "\\", "/"
            $objectForMake = "./$relativeDir/$baseName.o"
            $dependencyForMake = "./$relativeDir/$baseName.d"
            $targetForMake = "$relativeDir/$baseName.o"

            $cSrcLines.Add($sourceForMake) | Out-Null
            $objLines.Add($objectForMake) | Out-Null
            $depLines.Add($dependencyForMake) | Out-Null
            $ruleLines.Add("$targetForMake`: $sourceForMake $relativeDir/subdir.mk") | Out-Null
            $ruleLines.Add($MainCompileLine) | Out-Null
            $ruleLines.Add("") | Out-Null

            foreach ($suffix in @(".cyclo", ".d", ".o", ".su")) {
                $cleanFiles.Add("./$relativeDir/$baseName$suffix") | Out-Null
            }
        }

        $cleanTarget = "clean-" + ($relativeDir -replace "/", "-2f-")
        $subdirText = @(
            "################################################################################",
            "# Generated by tools/build_n647_app.ps1 when CubeIDE omits a local BSP source directory.",
            "################################################################################",
            "",
            "C_SRCS += \",
            (($cSrcLines | ForEach-Object { "$_ \" }) -join "`r`n").TrimEnd(" \"),
            "",
            "OBJS += \",
            (($objLines | ForEach-Object { "$_ \" }) -join "`r`n").TrimEnd(" \"),
            "",
            "C_DEPS += \",
            (($depLines | ForEach-Object { "$_ \" }) -join "`r`n").TrimEnd(" \"),
            "",
            ($ruleLines -join "`r`n"),
            "clean: $cleanTarget",
            "",
            "$cleanTarget`:",
            ("`t-`$(RM) " + ($cleanFiles -join " ")),
            "",
            ".PHONY: $cleanTarget",
            ""
        ) -join "`r`n"

        Write-TextFileNoBom -Path $subdirMk -Text $subdirText
        Write-Host ("Added generated local BSP source list for {0}: {1} file(s)." -f $relativeDir, $sources.Count)
    }
}

function Repair-GeneratedMakefileSubdirIncludes {
    param([string]$ConfigDir)

    $makefile = Join-Path $ConfigDir "makefile"
    $sourcesMk = Join-Path $ConfigDir "sources.mk"
    if (-not (Test-Path -LiteralPath $makefile -PathType Leaf) -or
        -not (Test-Path -LiteralPath $sourcesMk -PathType Leaf)) {
        return
    }

    $subdirMakefiles = Get-GeneratedSubdirMakefiles -ConfigDir $ConfigDir
    $subdirDirs = @($subdirMakefiles | ForEach-Object {
        (Get-ConfigRelativeMakePath -ConfigDir $ConfigDir -Path $_.FullName) -replace "/subdir\.mk$", ""
    })

    $makeLines = New-Object System.Collections.Generic.List[string]
    $originalMakeLines = @(Get-Content -LiteralPath $makefile)
    $sourceIncludeIndex = [Array]::FindIndex($originalMakeLines, [Predicate[string]]{ param($line) $line -eq "-include sources.mk" })
    $objectsIncludeIndex = [Array]::FindIndex($originalMakeLines, [Predicate[string]]{ param($line) $line -eq "-include objects.mk" })

    if ($sourceIncludeIndex -lt 0 -or $objectsIncludeIndex -lt 0 -or $objectsIncludeIndex -le $sourceIncludeIndex) {
        throw "Cannot locate generated include block in $makefile"
    }

    for ($i = 0; $i -le $sourceIncludeIndex; $i++) {
        $makeLines.Add($originalMakeLines[$i]) | Out-Null
    }
    foreach ($subdirDir in $subdirDirs) {
        $makeLines.Add("-include $subdirDir/subdir.mk") | Out-Null
    }
    for ($i = $objectsIncludeIndex; $i -lt $originalMakeLines.Count; $i++) {
        $makeLines.Add($originalMakeLines[$i]) | Out-Null
    }

    $newMakeText = ($makeLines -join "`r`n") + "`r`n"
    $oldMakeText = Get-Content -LiteralPath $makefile -Raw
    if ($newMakeText -ne $oldMakeText) {
        Write-TextFileNoBom -Path $makefile -Text $newMakeText
        Write-Host ("Repaired generated makefile include list for {0}." -f (Split-Path -Leaf $ConfigDir))
    }

    $sourceLines = New-Object System.Collections.Generic.List[string]
    $originalSourceLines = @(Get-Content -LiteralPath $sourcesMk)
    $subdirsIndex = [Array]::FindIndex($originalSourceLines, [Predicate[string]]{ param($line) $line -like "SUBDIRS :=*" })
    if ($subdirsIndex -lt 0) {
        return
    }

    $subdirsEndIndex = $subdirsIndex + 1
    while ($subdirsEndIndex -lt $originalSourceLines.Count -and
        -not [string]::IsNullOrWhiteSpace($originalSourceLines[$subdirsEndIndex])) {
        $subdirsEndIndex++
    }

    for ($i = 0; $i -lt $subdirsIndex; $i++) {
        $sourceLines.Add($originalSourceLines[$i]) | Out-Null
    }
    $sourceLines.Add("SUBDIRS := \") | Out-Null
    foreach ($subdirDir in $subdirDirs) {
        $sourceLines.Add("$subdirDir \") | Out-Null
    }
    for ($i = $subdirsEndIndex; $i -lt $originalSourceLines.Count; $i++) {
        $sourceLines.Add($originalSourceLines[$i]) | Out-Null
    }

    $newSourcesText = ($sourceLines -join "`r`n") + "`r`n"
    $oldSourcesText = Get-Content -LiteralPath $sourcesMk -Raw
    if ($newSourcesText -ne $oldSourcesText) {
        Write-TextFileNoBom -Path $sourcesMk -Text $newSourcesText
        Write-Host ("Repaired generated sources.mk SUBDIRS for {0}." -f (Split-Path -Leaf $ConfigDir))
    }
}

function Repair-GeneratedObjectsListFromSubdirMakefiles {
    param([string]$ConfigDir)

    $objectsList = Join-Path $ConfigDir "objects.list"
    if (-not (Test-Path -LiteralPath $objectsList -PathType Leaf)) {
        return
    }

    $objectLines = New-Object System.Collections.Generic.List[string]
    foreach ($subdirMk in Get-GeneratedSubdirMakefiles -ConfigDir $ConfigDir) {
        foreach ($objectEntry in Get-MakeVariableAppendEntries -Path $subdirMk.FullName -Variable "OBJS") {
            $quotedEntry = "`"$objectEntry`""
            if (-not ($objectLines -contains $quotedEntry)) {
                $objectLines.Add($quotedEntry) | Out-Null
            }
        }
    }

    if ($objectLines.Count -eq 0) {
        return
    }

    $newObjectsText = ($objectLines -join "`r`n") + "`r`n"
    $oldObjectsText = Get-Content -LiteralPath $objectsList -Raw
    if ($newObjectsText -ne $oldObjectsText) {
        Write-TextFileNoBom -Path $objectsList -Text $newObjectsText
        Write-Host ("Rebuilt objects.list from generated subdir.mk files for {0}: {1} object(s)." -f (Split-Path -Leaf $ConfigDir), $objectLines.Count)
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

    $mainCompileLine = Get-MainCompileLine -ConfigDir $ConfigDir -CoreSubdirText $subdirText

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

        $coreSubdirMk = Join-Path $configDir "Application\User\Core\subdir.mk"
        if (-not (Test-Path -LiteralPath $coreSubdirMk -PathType Leaf)) {
            throw "$config Core source list not found. Generate/build the CubeIDE project once first: $coreSubdirMk"
        }
        $mainCompileLine = Get-MainCompileLine -ConfigDir $configDir -CoreSubdirText (Get-Content -LiteralPath $coreSubdirMk -Raw)

        Add-MissingLocalBspSubdirMakefiles -ConfigDir $configDir -MainCompileLine $mainCompileLine
        Repair-GeneratedMakefileSubdirIncludes -ConfigDir $configDir
        Repair-GeneratedCoreSourceList -ConfigDir $configDir
        Repair-GeneratedMakefileSubdirIncludes -ConfigDir $configDir
        Repair-GeneratedObjectsListFromSubdirMakefiles -ConfigDir $configDir

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
