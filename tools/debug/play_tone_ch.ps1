# Stereo variant of play_tone.ps1 with per-channel routing, for the
# two-speaker localization calibration (one speaker on L, one on R).
# -Channel left  : tone on the LEFT channel only, right silent
# -Channel right : tone on the RIGHT channel only, left silent
# -Channel both  : tone on both (sanity checks)
param(
    [double]$FreqHz = 2000,
    [double]$Seconds = 6,
    [ValidateSet("left", "right", "both")] [string]$Channel = "both",
    [ValidateSet("sine", "noise")] [string]$Kind = "sine",
    [double]$Gain = 0.9
)

Add-Type -TypeDefinition @"
using System;
using System.IO;

public static class CalToneGenStereo
{
    public static void Generate(string path, string kind, double freqHz,
                                double seconds, double gain,
                                bool left, bool right)
    {
        const int rate = 48000;
        int n = (int)(rate * seconds);
        var rand = new Random(1234);
        using (var bw = new BinaryWriter(File.Create(path)))
        {
            bw.Write(System.Text.Encoding.ASCII.GetBytes("RIFF"));
            bw.Write(36 + n * 4);
            bw.Write(System.Text.Encoding.ASCII.GetBytes("WAVEfmt "));
            bw.Write(16);
            bw.Write((short)1);
            bw.Write((short)2);
            bw.Write(rate);
            bw.Write(rate * 4);
            bw.Write((short)4);
            bw.Write((short)16);
            bw.Write(System.Text.Encoding.ASCII.GetBytes("data"));
            bw.Write(n * 4);

            for (int i = 0; i < n; i++)
            {
                double v;
                if (kind == "noise")
                {
                    v = (rand.NextDouble() * 2.0 - 1.0) * 0.7;
                }
                else
                {
                    v = Math.Sin(2.0 * Math.PI * freqHz * i / rate);
                }
                double s = v * gain * 32767.0;
                if (s > 32767.0) { s = 32767.0; }
                if (s < -32767.0) { s = -32767.0; }
                bw.Write((short)(left ? s : 0.0));
                bw.Write((short)(right ? s : 0.0));
            }
        }
    }
}
"@ -ErrorAction SilentlyContinue

$wav = Join-Path $env:TEMP ("cal_st_{0}_{1}_{2}hz_{3}s_g{4}.wav" -f `
    $Channel, $Kind, [int]$FreqHz, [int]$Seconds, [int]($Gain * 100))

if (-not (Test-Path $wav)) {
    [CalToneGenStereo]::Generate($wav, $Kind, $FreqHz, $Seconds, $Gain,
                                 ($Channel -ne "right"), ($Channel -ne "left"))
}

Write-Host ("playing {0} {1} Hz on channel={2} for {3} s ..." -f `
    $Kind, $FreqHz, $Channel, $Seconds)
$player = New-Object System.Media.SoundPlayer $wav
$player.PlaySync()
Write-Host "done"
