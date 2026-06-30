[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\n647_touchgfx_common.ps1"

$info = Find-N647TouchGFX
Set-N647TouchGFXProcessPath -Info $info

$projectDir = Get-N647TouchGFXProjectDir
Push-Location $projectDir
try {
  & $info.Make -f simulator/gcc/Makefile -j8
  if ($LASTEXITCODE -ne 0) {
    throw "TouchGFX simulator build failed with exit code $LASTEXITCODE"
  }
} finally {
  Pop-Location
}

Write-Host "TouchGFX simulator built successfully."
