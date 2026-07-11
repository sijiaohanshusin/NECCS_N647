# Acoustic localization calibration loop.
# Plays a test tone, samples SRP telemetry, captures a screenshot.
param(
    [double]$FreqHz = 3000,
    # Long tone: the screenshot (GDB framebuffer dump) starts ~9 s into the
    # tone and halts the CPU 2-3 s later - it must still be playing then.
    [double]$ToneSeconds = 20,
    [int]$Samples = 3
)

$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$gdb = (Get-ChildItem "C:\ST\STM32CubeIDE_2.1.1\STM32CubeIDE\plugins" -Recurse -Filter "arm-none-eabi-gdb.exe" |
        Select-Object -First 1).FullName
$elf = Join-Path $root "NECCS_N647_App\STM32CubeIDE\Appli\Debug\NECCS_N647_App_Appli.elf"
$logDir = Join-Path $root "_debug_logs"
$stamp = Get-Date -Format "MMdd_HHmmss"
$report = Join-Path $logDir "acou_cal_$stamp.txt"

function Invoke-AcouSample {
    $out = & $gdb --batch -x (Join-Path $logDir "acou_sample.cmd") $elf 2>$null
    $snap = ($out | Select-String "SNAP").Line
    $cand = ($out | Select-String "CAND").Line
    [PSCustomObject]@{
        Snap = $snap
        Cand = $cand
    }
}

function Invoke-MicPeak {
    $out = & $gdb --batch -x (Join-Path $logDir "mic_levels.cmd") $elf 2>$null
    ($out | Select-String "PEAK").Line
}

"=== quiet baseline ===" | Tee-Object -FilePath $report
Invoke-MicPeak | Tee-Object -FilePath $report -Append
1..2 | ForEach-Object {
    $s = Invoke-AcouSample
    "$($s.Snap)" | Tee-Object -FilePath $report -Append
}

$toneJob = Start-Job {
    param($freq, $sec, $script)
    & $script -FreqHz $freq -Seconds $sec
} -ArgumentList $FreqHz, $ToneSeconds, (Join-Path $PSScriptRoot "play_tone.ps1")

Start-Sleep -Seconds 2
"=== tone ${FreqHz}Hz ===" | Tee-Object -FilePath $report -Append
Invoke-MicPeak | Tee-Object -FilePath $report -Append
1..$Samples | ForEach-Object {
    $s = Invoke-AcouSample
    "$($s.Snap)" | Tee-Object -FilePath $report -Append
    "$($s.Cand)" | Tee-Object -FilePath $report -Append
    Start-Sleep -Milliseconds 700
}
Invoke-MicPeak | Tee-Object -FilePath $report -Append

& (Join-Path $PSScriptRoot "n647.ps1") screenshot 2>&1 |
    Tee-Object -FilePath $report -Append

Wait-Job $toneJob | Out-Null
Remove-Job $toneJob -Force

"=== profile sweep ===" | Tee-Object -FilePath $report -Append
# Sweep order ends on 1 so the board is left in the standard profile.
foreach ($mode in 0, 2, 1) {
    $tpl = Get-Content (Join-Path $logDir "prof_set.cmd.tpl") -Raw
    $cmd = $tpl -replace 's_requested_profile', 's_requested_mode' -replace 'PROFVAL', "$mode"
    Set-Content (Join-Path $logDir "prof_set.cmd") $cmd -Encoding ascii
    & $gdb --batch -x (Join-Path $logDir "prof_set.cmd") $elf 2>$null | Out-Null
    Start-Sleep -Seconds 3
    $p = & $gdb --batch -x (Join-Path $logDir "prof_read.cmd") $elf 2>$null | Select-String "PROF"
    "mode=$mode $p" | Tee-Object -FilePath $report -Append
}

Write-Host "report: $report"
