# Play a sine tone (or noise/sweep) on the default audio device for
# acoustic calibration. Synchronous; returns when playback ends.
# WAVs are generated in compiled C# (instant) and cached in TEMP, so
# playback starts immediately even for long durations.
param(
    [double]$FreqHz = 2000,
    [double]$Seconds = 6,
    [ValidateSet("sine", "noise", "sweep")] [string]$Kind = "sine",
    [double]$Gain = 0.9
)

Add-Type -TypeDefinition @"
using System;
using System.IO;

public static class CalToneGen
{
    public static void Generate(string path, string kind, double freqHz,
                                double seconds, double gain)
    {
        const int rate = 48000;
        int n = (int)(rate * seconds);
        var rand = new Random(1234);
        using (var bw = new BinaryWriter(File.Create(path)))
        {
            bw.Write(System.Text.Encoding.ASCII.GetBytes("RIFF"));
            bw.Write(36 + n * 2);
            bw.Write(System.Text.Encoding.ASCII.GetBytes("WAVEfmt "));
            bw.Write(16);
            bw.Write((short)1);
            bw.Write((short)1);
            bw.Write(rate);
            bw.Write(rate * 2);
            bw.Write((short)2);
            bw.Write((short)16);
            bw.Write(System.Text.Encoding.ASCII.GetBytes("data"));
            bw.Write(n * 2);

            for (int i = 0; i < n; i++)
            {
                double v;
                if (kind == "noise")
                {
                    v = (rand.NextDouble() * 2.0 - 1.0) * 0.7;
                }
                else if (kind == "sweep")
                {
                    double frac = (double)i / n;
                    double f = 500.0 * Math.Pow(16.0, frac);
                    v = Math.Sin(2.0 * Math.PI * f * i / rate);
                }
                else
                {
                    v = Math.Sin(2.0 * Math.PI * freqHz * i / rate);
                }
                double s = v * gain * 32767.0;
                if (s > 32767.0) { s = 32767.0; }
                if (s < -32767.0) { s = -32767.0; }
                bw.Write((short)s);
            }
        }
    }
}
"@ -ErrorAction SilentlyContinue

$wav = Join-Path $env:TEMP ("cal_{0}_{1}hz_{2}s_g{3}.wav" -f `
    $Kind, [int]$FreqHz, [int]$Seconds, [int]($Gain * 100))

if (-not (Test-Path $wav)) {
    [CalToneGen]::Generate($wav, $Kind, $FreqHz, $Seconds, $Gain)
}

Write-Host ("playing {0} {1} Hz for {2} s ..." -f $Kind, $FreqHz, $Seconds)
$player = New-Object System.Media.SoundPlayer $wav
$player.PlaySync()
Write-Host "done"
