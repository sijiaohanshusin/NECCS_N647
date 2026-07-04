[CmdletBinding()]
param(
    [string]$Python = "python",
    [string]$StEdgeAi = $env:STEDGEAI_EXE,
    [switch]$SpecOnly,
    [switch]$SkipStEdgeAi
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$npuDir = Join-Path $PSScriptRoot "npu"
$outputDir = Join-Path $npuDir "generated"
$stAiOutput = Join-Path $npuDir "st_ai_output"
$stAiWorkspace = Join-Path $npuDir "st_ai_ws"
$memoryPool = Join-Path $npuDir "neccs_n647_stm32n6.mpool"
$assetScript = Join-Path $PSScriptRoot "generate_npu_heatmap_assets.py"
$onnxPath = Join-Path $outputDir "npu_heatmap_qlinear_matmul.onnx"

if ([string]::IsNullOrWhiteSpace($StEdgeAi)) {
    $candidate = "C:\ST\STEdgeAI\4.0\Utilities\windows\stedgeai.exe"
    if (Test-Path -LiteralPath $candidate -PathType Leaf) {
        $StEdgeAi = $candidate
    }
}

New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

$assetArgs = @($assetScript, "--output-dir", $outputDir)
if (-not $SpecOnly) {
    $assetArgs += "--emit-onnx"
}

& $Python @assetArgs
if ($LASTEXITCODE -ne 0) {
    throw "NPU heatmap asset generation failed. For ONNX output, install the optional Python 'onnx' package or rerun with -SpecOnly."
}

if ($SpecOnly -or $SkipStEdgeAi) {
    Write-Host "NPU heatmap spec generated. ST Edge AI generation skipped."
    exit 0
}

if (-not (Test-Path -LiteralPath $StEdgeAi -PathType Leaf)) {
    throw "stedgeai.exe not found. Set STEDGEAI_EXE or install ST Edge AI Core 4.0."
}

if (-not (Test-Path -LiteralPath $onnxPath -PathType Leaf)) {
    throw "ONNX model was not generated: $onnxPath"
}

if (-not (Test-Path -LiteralPath $memoryPool -PathType Leaf)) {
    throw "NPU memory-pool descriptor not found: $memoryPool"
}

New-Item -ItemType Directory -Force -Path $stAiOutput | Out-Null
New-Item -ItemType Directory -Force -Path $stAiWorkspace | Out-Null

& $StEdgeAi analyze `
    --target stm32n6 `
    --st-neural-art `
    --model $onnxPath `
    --input-data-type int8 `
    --output-data-type int8 `
    --memory-pool $memoryPool `
    --workspace $stAiWorkspace `
    --output $stAiOutput

if ($LASTEXITCODE -ne 0) {
    throw "ST Edge AI analyze failed for $onnxPath"
}

& $StEdgeAi generate `
    --target stm32n6 `
    --st-neural-art `
    --model $onnxPath `
    --input-data-type int8 `
    --output-data-type int8 `
    --optimization time `
    --memory-pool $memoryPool `
    --workspace $stAiWorkspace `
    --output $stAiOutput

if ($LASTEXITCODE -ne 0) {
    throw "ST Edge AI generate failed for $onnxPath"
}

Write-Host "NPU heatmap model generated under $stAiOutput"
