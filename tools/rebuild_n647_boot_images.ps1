[CmdletBinding()]
param(
    [int]$Jobs = 8,

    [ValidateSet("VendorPrebuilt", "Rebuild")]
    [string]$FsblSource = "VendorPrebuilt",

    [string]$CubeIdeRoot = $env:STM32CUBEIDE_ROOT
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$appBuildScript = Join-Path $PSScriptRoot "build_n647_app.ps1"
$vendorPrebuiltFsbl = Join-Path $root "_vendor_fsbl_ref\Binary\fsbl.hex"
$fsblRelease = Join-Path $root "_vendor_software_ref\FSBL\MX25UM25645G_W958D8NBYA5I_Example\STM32CubeIDE\FSBL\Release"
$fsblBinaryDir = Join-Path $root "_vendor_software_ref\FSBL\MX25UM25645G_W958D8NBYA5I_Example\Binary"
$appHex = Join-Path $root "NECCS_N647_App\Binary\appli.hex"
$flashImages = Join-Path $root "_flash_images"

function Resolve-CubeIdeRoot {
    param([string]$RequestedRoot)

    if (-not [string]::IsNullOrWhiteSpace($RequestedRoot)) {
        return $RequestedRoot
    }

    $install = Get-ChildItem -LiteralPath "C:\ST" -Directory -Filter "STM32CubeIDE_*" -ErrorAction SilentlyContinue |
        Sort-Object Name -Descending |
        Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName "STM32CubeIDE\headless-build.bat") } |
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

function Invoke-ReleaseBuild {
    param(
        [string]$BuildDir,
        [string]$MakeTool
    )

    Push-Location $BuildDir
    try {
        & $MakeTool "-j$Jobs" main-build
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed in $BuildDir"
        }
    } finally {
        Pop-Location
    }
}

function Assert-FsblTrustedLayout {
    param([string]$TrustedImage)

    $bytes = [System.IO.File]::ReadAllBytes($TrustedImage)
    if ($bytes.Length -lt 0x408) {
        throw "FSBL trusted image is too small to contain a payload at offset 0x400: $TrustedImage"
    }

    $sp = [BitConverter]::ToUInt32($bytes, 0x400)
    $pc = [BitConverter]::ToUInt32($bytes, 0x404)
    $pcAddress = $pc -band 0xFFFFFFFE

    if ($sp -lt 0x34100000 -or $sp -gt 0x34210000 -or
        $pcAddress -lt 0x34180000 -or $pcAddress -gt 0x341C0000 -or
        ($pc -band 1) -ne 1) {
        throw ("FSBL trusted image layout check failed. Expected vector table at offset 0x400, got SP=0x{0:X8}, PC=0x{1:X8}." -f $sp, $pc)
    }
}

function Write-CombinedIntelHex {
    param(
        [string[]]$InputHexes,
        [string]$OutputHex
    )

    $records = New-Object System.Collections.Generic.List[string]

    foreach ($hex in $InputHexes) {
        foreach ($line in Get-Content -LiteralPath $hex) {
            if ([string]::IsNullOrWhiteSpace($line)) {
                continue
            }

            if ($line.Length -ge 9) {
                $recordType = $line.Substring(7, 2)
                if ($recordType -eq "01" -or $recordType -eq "05") {
                    continue
                }
            }

            $records.Add($line)
        }
    }

    $records.Add(":00000001FF")
    Set-Content -LiteralPath $OutputHex -Encoding ascii -Value $records
}

$CubeIdeRoot = Resolve-CubeIdeRoot -RequestedRoot $CubeIdeRoot
New-Item -ItemType Directory -Path $flashImages -Force | Out-Null

if ($FsblSource -eq "VendorPrebuilt") {
    if (-not (Test-Path -LiteralPath $vendorPrebuiltFsbl -PathType Leaf)) {
        throw "Vendor prebuilt FSBL was not found: $vendorPrebuiltFsbl"
    }

    Write-Host "Using the validated vendor prebuilt FSBL..."
    $fsblHex = $vendorPrebuiltFsbl
} else {
    $gnuTools = Find-CubeIdeToolDirectory -IdeRoot $CubeIdeRoot -PluginPattern "com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32*.win32_*"
    $makeTools = Find-CubeIdeToolDirectory -IdeRoot $CubeIdeRoot -PluginPattern "com.st.stm32cube.ide.mcu.externaltools.make.win32_*"
    $make = Join-Path $makeTools "make.exe"
    $objcopy = Join-Path $gnuTools "arm-none-eabi-objcopy.exe"
    $signingTool = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_SigningTool_CLI.exe"

    foreach ($tool in @($make, $objcopy, $signingTool)) {
        if (-not (Test-Path -LiteralPath $tool -PathType Leaf)) {
            throw "Required tool not found: $tool"
        }
    }

    $env:Path = "$gnuTools;$makeTools;$env:Path"
    New-Item -ItemType Directory -Path $fsblBinaryDir -Force | Out-Null

    Write-Host "Building FSBL Release..."
    Invoke-ReleaseBuild -BuildDir $fsblRelease -MakeTool $make

    $fsblBin = Join-Path $fsblRelease "MX25UM25645G_W958D8NBYA5I_Example_FSBL.bin"
    $fsblTrusted = Join-Path $fsblRelease "MX25UM25645G_W958D8NBYA5I_Example_FSBL-trusted.bin"
    $fsblHex = Join-Path $fsblBinaryDir "fsbl.hex"

    if (-not (Test-Path -LiteralPath $fsblBin -PathType Leaf)) {
        throw "FSBL binary was not generated: $fsblBin"
    }

    Remove-Item -LiteralPath $fsblTrusted -Force -ErrorAction SilentlyContinue
    & $signingTool -bin $fsblBin -nk -of 0x80000000 -t fsbl -o $fsblTrusted -hv 2.3 -align -dump $fsblTrusted
    if ($LASTEXITCODE -ne 0) {
        throw "FSBL signing failed."
    }

    Assert-FsblTrustedLayout -TrustedImage $fsblTrusted

    & $objcopy -I binary $fsblTrusted --change-addresses 0x70000000 -O ihex $fsblHex
    if ($LASTEXITCODE -ne 0) {
        throw "FSBL HEX conversion failed."
    }
}

Write-Host "Building the current NECCS_N647_App Release..."
& $appBuildScript -Configuration Release -CubeIdeRoot $CubeIdeRoot
if ($LASTEXITCODE -ne 0) {
    throw "NECCS_N647_App Release build failed."
}

if (-not (Test-Path -LiteralPath $appHex -PathType Leaf)) {
    throw "Application HEX was not generated: $appHex"
}

$flashFsbl = Join-Path $flashImages "fsbl.hex"
$flashApp = Join-Path $flashImages "appli.hex"
$flashBundle = Join-Path $flashImages "n647_boot_bundle.hex"
Copy-Item -LiteralPath $fsblHex -Destination $flashFsbl -Force
Copy-Item -LiteralPath $appHex -Destination $flashApp -Force
Write-CombinedIntelHex -InputHexes @($flashFsbl, $flashApp) -OutputHex $flashBundle

$fsblHead = Get-Content -LiteralPath $flashFsbl -TotalCount 2
$appHead = Get-Content -LiteralPath $flashApp -TotalCount 2

if ($fsblHead[0] -ne ":0200000470008A" -or $fsblHead[1] -notlike "*53544D32*") {
    throw "FSBL HEX header check failed. Expected STM2 header at 0x70000000."
}

if ($appHead[0] -ne ":0200000470107A" -or $appHead[1] -notlike ":10040000*") {
    throw "Application HEX address check failed. Expected vector table at 0x70100400."
}

Write-Host ""
Write-Host "Boot images are ready:"
Get-Item -LiteralPath $flashFsbl, $flashApp, $flashBundle | Select-Object FullName, Length, LastWriteTime
