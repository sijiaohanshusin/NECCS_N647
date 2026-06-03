$ErrorActionPreference = "Stop"

$src = "C:\Program Files\SEGGER\JLink_V946\JLink_x64.dll"
$dst1 = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\JLink_x64.dll"
$dst2 = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\api\lib\JLink_x64.dll"
$bak1 = "$dst1.bak_7.92o"
$bak2 = "$dst2.bak_7.92o"

Write-Host "Replacing STM32CubeProgrammer bundled J-Link DLLs with SEGGER 9.46..."

if (!(Test-Path -LiteralPath $src)) {
    throw "Source DLL not found: $src"
}

if (!(Test-Path -LiteralPath $bak1)) {
    Copy-Item -LiteralPath $dst1 -Destination $bak1
    Write-Host "Created backup: $bak1"
}

if (!(Test-Path -LiteralPath $bak2)) {
    Copy-Item -LiteralPath $dst2 -Destination $bak2
    Write-Host "Created backup: $bak2"
}

Copy-Item -LiteralPath $src -Destination $dst1 -Force
Copy-Item -LiteralPath $src -Destination $dst2 -Force

Write-Host ""
Write-Host "Current versions:"
(Get-Item -LiteralPath $dst1).VersionInfo | Format-List FileVersion, ProductVersion, FileDescription
(Get-Item -LiteralPath $dst2).VersionInfo | Format-List FileVersion, ProductVersion, FileDescription

Write-Host ""
Write-Host "Done. Restart STM32CubeProgrammer before reconnecting with J-Link."
