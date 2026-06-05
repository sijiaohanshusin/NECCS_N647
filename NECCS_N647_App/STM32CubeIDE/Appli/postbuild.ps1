$ErrorActionPreference = "Stop"

$projectDir = Split-Path -Parent $PSCommandPath
$projectRoot = Resolve-Path (Join-Path $projectDir "..\..")
$binaryDir = Join-Path $projectRoot "Binary"
$outputHex = Join-Path $binaryDir "appli.hex"

$bin = Get-ChildItem -LiteralPath $projectDir -Recurse -Filter "*_Appli.bin" |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if ($null -eq $bin) {
    throw "Cannot find *_Appli.bin under $projectDir"
}

New-Item -ItemType Directory -Force -Path $binaryDir | Out-Null

& arm-none-eabi-objcopy -I binary $bin.FullName --change-addresses 0x70100400 -O ihex $outputHex
if ($LASTEXITCODE -ne 0) {
    throw "arm-none-eabi-objcopy failed with exit code $LASTEXITCODE"
}

Write-Host "Generated $outputHex from $($bin.FullName)"
