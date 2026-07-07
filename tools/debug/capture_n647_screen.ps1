[CmdletBinding()]
param(
    # Output PNG path. Default: _debug_logs\screen_<timestamp>.png
    [string]$OutPng,
    # Keep separate _ui/_cam layer PNGs alongside the composite.
    [switch]$KeepLayers,
    # Reuse an already-running OpenOCD on this GDB port instead of starting one.
    [switch]$NoStartOpenOcd,
    [int]$GdbPort = 3333,
    [string]$Python = "python"
)

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "n647_debug_env.ps1")

$envInfo = Get-N647DebugEnv
if ($envInfo.Problems.Count -gt 0) {
    Write-N647DebugEnv -EnvInfo $envInfo
    throw "N647 debug environment is incomplete."
}

$stamp = Get-Date -Format "yyyyMMdd_HHmmss"
if ([string]::IsNullOrWhiteSpace($OutPng)) {
    $OutPng = Join-Path $envInfo.LogRoot "screen_$stamp.png"
}
$OutPng = [System.IO.Path]::GetFullPath($OutPng)

$uiBin = Join-Path $envInfo.LogRoot "fb_ui_$stamp.bin"
$camBin = Join-Path $envInfo.LogRoot "fb_cam_$stamp.bin"
$gdbCmd = Join-Path $envInfo.LogRoot "gdb_capture_$stamp.cmd"

function ConvertTo-N647GdbPath {
    param([string]$Path)
    return ([System.IO.Path]::GetFullPath($Path) -replace "\\", "/")
}

$gdbElf = ConvertTo-N647GdbPath -Path $envInfo.DebugElf
$gdbUiBin = ConvertTo-N647GdbPath -Path $uiBin
$gdbCamBin = ConvertTo-N647GdbPath -Path $camBin

# Halt briefly, dump the UI framebuffer and (when the camera layer is live)
# the camera compose buffer, then resume. Total halt time is ~1-2 s.
$commands = @(
    "set confirm off",
    "set pagination off",
    "set mem inaccessible-by-default off",
    "file `"$gdbElf`"",
    "target extended-remote localhost:$GdbPort",
    "monitor halt",
    "printf `"ui_fb=0x%08x l1_cr=0x%08x l1_cfbar=0x%08x flags=0x%x\\n`", g_app_camera_ui_fb_addr, g_app_camera_ltdc_layer1_cr, g_app_camera_ltdc_layer1_cfbar, g_app_camera_display_flags",
    "dump binary memory $gdbUiBin g_app_camera_ui_fb_addr (g_app_camera_ui_fb_addr + 1024*600*2)",
    "printf `"tag=UI_DUMPED\\n`"",
    "if (g_app_camera_ltdc_layer1_cr & 1) && (g_app_camera_ltdc_layer1_cfbar != 0)",
    "  dump binary memory $gdbCamBin g_app_camera_ltdc_layer1_cfbar (g_app_camera_ltdc_layer1_cfbar + 640*480*2)",
    "  printf `"tag=CAM_DUMPED\\n`"",
    "end",
    "monitor resume",
    "quit"
)
Set-Content -LiteralPath $gdbCmd -Value $commands -Encoding ASCII

$openOcdSession = $null
$startedOpenOcd = $false

try {
    if (-not $NoStartOpenOcd) {
        $startOpenOcd = Join-Path $PSScriptRoot "start_n647_openocd.ps1"
        $openOcdSession = & $startOpenOcd -GdbPort $GdbPort -Wait
        $startedOpenOcd = $true
    }

    $oldEap = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        & $envInfo.GdbExe --batch -q -x $gdbCmd 2>&1 | ForEach-Object { "$_" }
        $gdbExit = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $oldEap
    }
    if ($gdbExit -ne 0) {
        throw "GDB capture failed with exit code $gdbExit."
    }
} finally {
    if ($startedOpenOcd -and $null -ne $openOcdSession.ProcessId) {
        Stop-Process -Id $openOcdSession.ProcessId -ErrorAction SilentlyContinue
    }
}

if (-not (Test-Path -LiteralPath $uiBin -PathType Leaf)) {
    throw "UI framebuffer dump missing: $uiBin"
}

$composeArgs = @(
    (Join-Path $PSScriptRoot "fb_compose_png.py"),
    "--ui", $uiBin,
    "--out", $OutPng
)
if (Test-Path -LiteralPath $camBin -PathType Leaf) {
    $composeArgs += @("--cam", $camBin)
}
if ($KeepLayers) {
    $composeArgs += "--keep-layers"
}

& $Python @composeArgs
if ($LASTEXITCODE -ne 0) {
    throw "PNG composition failed with exit code $LASTEXITCODE."
}

Remove-Item -LiteralPath $uiBin -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $camBin -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $gdbCmd -ErrorAction SilentlyContinue

Write-Host "Screen captured: $OutPng"
[pscustomobject]@{ OutPng = $OutPng }
