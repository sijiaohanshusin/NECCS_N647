# One-shot NPU dataset collection: plays each class sound on the PC while
# collect_spectra.py grabs labeled spectrum windows from the live board over
# the OpenOCD telnet port. Board must be running the Debug build in Wide32
# mode with OpenOCD up (n647.ps1 flash-debug leaves it that way).
param(
    [int]$Seconds = 65,
    [string]$SoundDir = "D:\Project\NECCS\Program\NECCS_N647\_debug_logs\npu_sounds"
)

$repo = "D:\Project\NECCS\Program\NECCS_N647"
$collector = Join-Path $repo "tools\npu\collect_spectra.py"

function Collect-WithWav([string]$label, [string]$wav) {
    Write-Host "=== $label ==="
    $player = New-Object System.Media.SoundPlayer $wav
    $player.PlayLooping()
    Start-Sleep -Seconds 2   # let the mics settle on the new sound
    python $collector --label $label --seconds $Seconds
    $player.Stop()
    Start-Sleep -Seconds 1
}

# 1. quiet room -> listening
Write-Host "=== listening (silence) ==="
python $collector --label listening --seconds $Seconds

# 2-5. synthesized characteristic sounds
Collect-WithWav "gas_leak" (Join-Path $SoundDir "gas_leak.wav")
Collect-WithWav "bearing"  (Join-Path $SoundDir "bearing.wav")
Collect-WithWav "arc"      (Join-Path $SoundDir "arc.wav")
Collect-WithWav "impact"   (Join-Path $SoundDir "impact.wav")

# 6. ambient = real speech via Windows TTS looping in the background.
# ASCII text only: this file is read in the system codepage by PowerShell 5
# and non-ASCII literals get mangled. Spectrally, any language works.
Write-Host "=== ambient (TTS speech) ==="
$tts = Start-Job {
    Add-Type -AssemblyName System.Speech
    $s = New-Object System.Speech.Synthesis.SpeechSynthesizer
    $s.Rate = 0
    $text = "The acoustic imaging camera is collecting ambient speech samples. " +
            "This voice segment trains the ambient sound category. The device uses " +
            "a thirty two microphone array to localize sound sources in real time."
    while ($true) { $s.Speak($text) }
}
Start-Sleep -Seconds 2
python $collector --label ambient --seconds $Seconds
Stop-Job $tts; Remove-Job $tts -Force

Write-Host "collection complete"
Get-ChildItem (Join-Path $repo "tools\npu\dataset") -Directory | ForEach-Object {
    "{0,-10} {1}" -f $_.Name, (Get-ChildItem $_.FullName -Filter *.npy).Count
}
