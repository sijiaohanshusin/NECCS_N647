Set-StrictMode -Version Latest

function Get-N647RepoRoot {
  $scriptDir = Split-Path -Parent $PSCommandPath
  return (Resolve-Path (Join-Path $scriptDir "..\..")).Path
}

function Get-N647TouchGFXCandidates {
  $candidates = New-Object System.Collections.Generic.List[string]

  if ($env:TOUCHGFX_4_26_1_ROOT) {
    $candidates.Add($env:TOUCHGFX_4_26_1_ROOT)
  }

  $candidates.Add("C:\ST\TouchGFX\4.26.1")

  if ($env:TEMP) {
    $candidates.Add((Join-Path $env:TEMP "touchgfx_4_26_1_extract\TouchGFX\4.26.1"))
  }

  $repoPack = Join-Path $env:USERPROFILE "STM32Cube\Repository\Packs\STMicroelectronics\X-CUBE-TOUCHGFX\4.26.1"
  $candidates.Add($repoPack)

  return $candidates | Where-Object { $_ -and (Test-Path $_) } | Select-Object -Unique
}

function Test-N647TouchGFXRoot {
  param(
    [Parameter(Mandatory = $true)][string]$Root
  )

  $designer = Join-Path $Root "designer\TouchGFXDesigner-4.26.1.exe"
  $framework = Join-Path $Root "touchgfx\framework"
  $make = Join-Path $Root "env\MinGW\msys\1.0\bin\make.exe"
  $ruby = Join-Path $Root "env\MinGW\msys\1.0\Ruby30-x64\bin\ruby.exe"
  $fontconvert = Join-Path $Root "touchgfx\framework\tools\fontconvert\build\win\fontconvert.out"
  $imageconvert = Join-Path $Root "touchgfx\framework\tools\imageconvert\build\win\imageconvert.out"

  return [pscustomobject]@{
    Root = $Root
    IsComplete = ((Test-Path $designer) -and (Test-Path $framework) -and (Test-Path $make) -and (Test-Path $ruby) -and (Test-Path $fontconvert) -and (Test-Path $imageconvert))
    Designer = $designer
    Framework = $framework
    Make = $make
    Ruby = $ruby
    FontConvert = $fontconvert
    ImageConvert = $imageconvert
  }
}

function Find-N647TouchGFX {
  foreach ($candidate in Get-N647TouchGFXCandidates) {
    $info = Test-N647TouchGFXRoot -Root $candidate
    if ($info.IsComplete) {
      return $info
    }
  }

  $checked = Get-N647TouchGFXCandidates | ForEach-Object { "  - $_" }
  throw "TouchGFX 4.26.1 toolchain not found. Checked:`n$($checked -join "`n")"
}

function Set-N647TouchGFXProcessPath {
  param(
    [Parameter(Mandatory = $true)]$Info
  )

  $paths = @(
    (Join-Path $Info.Root "env\MinGW\msys\1.0\Ruby30-x64\bin"),
    (Join-Path $Info.Root "env\MinGW\msys\1.0\bin"),
    (Join-Path $Info.Root "env\MinGW\bin")
  )

  $env:PATH = (($paths + @($env:PATH)) -join ";")
}

function Get-N647TouchGFXProjectDir {
  $repoRoot = Get-N647RepoRoot
  return Join-Path $repoRoot "NECCS_N647_App\Appli\TouchGFX"
}
