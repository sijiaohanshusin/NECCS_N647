$ErrorActionPreference = "Stop"

$dst1 = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\JLink_x64.dll"
$dst2 = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\api\lib\JLink_x64.dll"
$bak1 = "$dst1.bak_7.92o"
$bak2 = "$dst2.bak_7.92o"

Write-Host "Restoring STM32CubeProgrammer bundled J-Link DLL backups..."

if (!(Test-Path -LiteralPath $bak1)) {
    throw "Backup not found: $bak1"
}

if (!(Test-Path -LiteralPath $bak2)) {
    throw "Backup not found: $bak2"
}

Copy-Item -LiteralPath $bak1 -Destination $dst1 -Force
Copy-Item -LiteralPath $bak2 -Destination $dst2 -Force

Write-Host ""
Write-Host "Current versions:"
(Get-Item -LiteralPath $dst1).VersionInfo | Format-List FileVersion, ProductVersion, FileDescription
(Get-Item -LiteralPath $dst2).VersionInfo | Format-List FileVersion, ProductVersion, FileDescription

Write-Host ""
Write-Host "Done. Restart STM32CubeProgrammer after restore."
