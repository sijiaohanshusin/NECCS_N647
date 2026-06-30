[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\n647_touchgfx_common.ps1"

$info = Find-N647TouchGFX
Set-N647TouchGFXProcessPath -Info $info

$projectDir = Get-N647TouchGFXProjectDir
Push-Location $projectDir
try {
  & $info.Make -f simulator/gcc/Makefile assets -j8
  if ($LASTEXITCODE -ne 0) {
    throw "TouchGFX asset generation failed with exit code $LASTEXITCODE"
  }

  $fontProvider = Join-Path $projectDir "generated\fonts\src\ApplicationFontProvider.cpp"
  $textKeys = Join-Path $projectDir "generated\texts\include\texts\TextKeysAndLanguages.hpp"
  $providerText = Get-Content $fontProvider -Raw
  $keysText = Get-Content $textKeys -Raw

  if ($providerText -notmatch "case\s+Typography::TITLE" -or
      $providerText -notmatch "case\s+Typography::BODY" -or
      $providerText -notmatch "case\s+Typography::SMALL") {
    throw "Generated ApplicationFontProvider does not expose the NECCS typographies."
  }
  if ($keysText -notmatch "T_APP_TITLE") {
    throw "Generated TextKeysAndLanguages.hpp does not contain NECCS text keys."
  }
} finally {
  Pop-Location
}

& "$PSScriptRoot\update_n647_touchgfx_project_links.ps1"

Write-Host "TouchGFX assets generated successfully from $projectDir"
