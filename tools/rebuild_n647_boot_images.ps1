param(
    [int]$Jobs = 8,
    [ValidateSet("VendorPrebuilt", "Rebuild")]
    [string]$FsblSource = "VendorPrebuilt"
)

$ErrorActionPreference = "Stop"

$root = Resolve-Path (Join-Path $PSScriptRoot "..")

$gnuTools = "C:\ST\STM32CubeIDE_2.1.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740\tools\bin"
$makeTools = "C:\ST\STM32CubeIDE_2.1.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.100.202601091506\tools\bin"
$signingTool = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_SigningTool_CLI.exe"

$make = Join-Path $makeTools "make.exe"
$objcopy = Join-Path $gnuTools "arm-none-eabi-objcopy.exe"

foreach ($tool in @($make, $objcopy, $signingTool)) {
    if (-not (Test-Path -LiteralPath $tool)) {
        throw "Required tool not found: $tool"
    }
}

$env:Path = "$gnuTools;$makeTools;$env:Path"

$fsblRelease = Join-Path $root "_vendor_software_ref\FSBL\MX25UM25645G_W958D8NBYA5I_Example\STM32CubeIDE\FSBL\Release"
$fsblBinaryDir = Join-Path $root "_vendor_software_ref\FSBL\MX25UM25645G_W958D8NBYA5I_Example\Binary"
$vendorPrebuiltFsbl = Join-Path $root "_vendor_fsbl_ref\Binary\fsbl.hex"
$appRelease = Join-Path $root "_vendor_software_ref\Projects\01_LED\STM32CubeIDE\Appli\Release"
$appBinaryDir = Join-Path $root "_vendor_software_ref\Projects\01_LED\Binary"
$flashImages = Join-Path $root "_flash_images"

foreach ($dir in @($fsblRelease, $fsblBinaryDir, $appRelease, $appBinaryDir, $flashImages)) {
    if (-not (Test-Path -LiteralPath $dir)) {
        New-Item -ItemType Directory -Path $dir | Out-Null
    }
}

function Invoke-ReleaseBuild {
    param(
        [string]$BuildDir
    )

    Push-Location $BuildDir
    try {
        & $make "-j$Jobs" main-build
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed in $BuildDir"
        }
    }
    finally {
        Pop-Location
    }
}

function Assert-FsblTrustedLayout {
    param(
        [string]$TrustedImage
    )

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

$fsblHex = Join-Path $fsblBinaryDir "fsbl.hex"

if ($FsblSource -eq "VendorPrebuilt") {
    if (-not (Test-Path -LiteralPath $vendorPrebuiltFsbl)) {
        throw "Vendor prebuilt FSBL was not found: $vendorPrebuiltFsbl"
    }

    Write-Host "Using vendor prebuilt FSBL..."
    Copy-Item -LiteralPath $vendorPrebuiltFsbl -Destination $fsblHex -Force
}
else {
    Write-Host "Building FSBL Release..."
    Invoke-ReleaseBuild -BuildDir $fsblRelease

    $fsblBin = Join-Path $fsblRelease "MX25UM25645G_W958D8NBYA5I_Example_FSBL.bin"
    $fsblTrusted = Join-Path $fsblRelease "MX25UM25645G_W958D8NBYA5I_Example_FSBL-trusted.bin"

    if (-not (Test-Path -LiteralPath $fsblBin)) {
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

Write-Host "Building 01_LED Appli Release..."
Invoke-ReleaseBuild -BuildDir $appRelease

$appBin = Join-Path $appRelease "01_LED_Appli.bin"
$appHex = Join-Path $appBinaryDir "appli.hex"

if (-not (Test-Path -LiteralPath $appBin)) {
    throw "Application binary was not generated: $appBin"
}

& $objcopy -I binary $appBin --change-addresses 0x70100400 -O ihex $appHex
if ($LASTEXITCODE -ne 0) {
    throw "Application HEX conversion failed."
}

$flashFsbl = Join-Path $flashImages "fsbl.hex"
$flashApp = Join-Path $flashImages "appli_01_LED.hex"
Copy-Item -LiteralPath $fsblHex -Destination $flashFsbl -Force
Copy-Item -LiteralPath $appHex -Destination $flashApp -Force

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
Get-Item -LiteralPath $flashFsbl, $flashApp | Select-Object FullName, Length, LastWriteTime
