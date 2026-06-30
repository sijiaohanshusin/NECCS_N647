[CmdletBinding()]
param(
  [switch]$Json
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\n647_touchgfx_common.ps1"

$info = Find-N647TouchGFX
Set-N647TouchGFXProcessPath -Info $info

$projectDir = Get-N647TouchGFXProjectDir
$appMk = Join-Path $projectDir "config\gcc\app.mk"
$touchgfxFile = Join-Path $projectDir "NECCS_N647_App.touchgfx"
$makeVersion = & $info.Make --version 2>$null | Select-Object -First 1
$rubyVersion = & $info.Ruby --version 2>$null
$gpp = Join-Path $info.Root "env\MinGW\bin\g++.exe"
$gppVersion = & $gpp --version 2>$null | Select-Object -First 1
$appMkText = Get-Content $appMk -Raw
$expectedTouchGFXPath = (($info.Root -replace "\\", "/") + "/touchgfx")

$result = [pscustomobject]@{
  TouchGFXVersion = "4.26.1"
  Root = $info.Root
  Designer = $info.Designer
  Framework = $info.Framework
  Make = $info.Make
  Ruby = $info.Ruby
  FontConvert = $info.FontConvert
  ImageConvert = $info.ImageConvert
  MakeVersion = $makeVersion
  RubyVersion = $rubyVersion
  GppVersion = $gppVersion
  Project = $touchgfxFile
  AppMkUsesStablePath = $appMkText.Contains($expectedTouchGFXPath)
  GeneratorPath = "simulator/gcc/Makefile assets"
}

if ($Json) {
  $result | ConvertTo-Json -Depth 4
} else {
  $result | Format-List
}

if (-not $result.AppMkUsesStablePath) {
  throw "config/gcc/app.mk does not point at $expectedTouchGFXPath"
}
