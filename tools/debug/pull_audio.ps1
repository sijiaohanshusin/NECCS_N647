# Pull an AUDnnnnn.WAV off the board's SD NAND over SWD (DEBUG builds only).
# Usage: .\tools\debug\pull_audio.ps1 -Index 15 [-OutDir _debug_logs\pulled]
param(
    [Parameter(Mandatory = $true)][int]$Index,
    [string]$OutDir = "_debug_logs\pulled"
)

$repo = "D:\Project\NECCS\Program\NECCS_N647"
$gdb = "C:\ST\STM32CubeIDE_2.1.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740\tools\bin\arm-none-eabi-gdb.exe"
$elf = Join-Path $repo "NECCS_N647_App\STM32CubeIDE\Appli\Debug\NECCS_N647_App_Appli.elf"

New-Item -ItemType Directory -Force (Join-Path $repo $OutDir) | Out-Null
$outFile = Join-Path $repo (Join-Path $OutDir ("AUD{0:D5}.WAV" -f $Index))
$chunkFile = Join-Path $repo "_debug_logs\pull_chunk.bin"
if (Test-Path $outFile) { Remove-Item $outFile }

$offset = 0
while ($true) {
    # Trigger one chunk read on the media thread.
    & $gdb --batch -ex "set confirm off" -ex "target extended-remote localhost:3333" `
        -ex "monitor halt" `
        -ex "set 'app_media.c'::g_app_media_pull_offset = $offset" `
        -ex "set 'app_media.c'::g_app_media_pull_result = 0" `
        -ex "set 'app_media.c'::g_app_media_pull_request = $Index" `
        -ex "monitor resume" -ex quit $elf 2>$null | Out-Null

    # Wait for the media thread's 200 ms tick to service it.
    Start-Sleep -Milliseconds 900

    $out = & $gdb --batch -ex "set confirm off" -ex "target extended-remote localhost:3333" `
        -ex "monitor halt" `
        -ex "printf \`"RES %u %u\n\`", 'app_media.c'::g_app_media_pull_result, 'app_media.c'::g_app_media_pull_busy" `
        -ex "dump binary memory $($chunkFile -replace '\\','/') &g_app_media_pull_buffer[0] &g_app_media_pull_buffer[65536]" `
        -ex "monitor resume" -ex quit $elf 2>$null | Select-String "^RES "
    if (-not $out) { Write-Host "no response, retrying"; continue }

    $parts = ($out.Line -replace "RES ", "") -split " "
    $result = [uint32]$parts[0]
    if ($result -eq 0xFFFFFFFFL) { Write-Host "read error at offset $offset"; break }
    if ($result -eq 0) { break }  # EOF

    $bytes = [System.IO.File]::ReadAllBytes($chunkFile)
    $stream = [System.IO.File]::Open($outFile, [System.IO.FileMode]::Append)
    $stream.Write($bytes, 0, [int]$result)
    $stream.Close()

    $offset += [int]$result
    Write-Host ("{0:N0} bytes" -f $offset)
    if ($result -lt 65536) { break }  # short read = EOF
}

Write-Host "saved: $outFile ($offset bytes)"
