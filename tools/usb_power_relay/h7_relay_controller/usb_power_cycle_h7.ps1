[CmdletBinding()]
param(
    [string]$Port,
    [ValidateSet("Cycle", "On", "Off", "Status")]
    [string]$Action = "Cycle",
    [int]$OffMs = 5000,
    [int]$Baud = 921600,
    [switch]$ListPorts,
    [switch]$AssertDtrRts,
    [switch]$Quiet
)

$ErrorActionPreference = "Stop"

function Get-SerialPortCandidates {
    $ports = @()

    try {
        $ports += Get-CimInstance Win32_SerialPort -ErrorAction SilentlyContinue |
            Select-Object DeviceID, Name, Description, PNPDeviceID
    } catch {
    }

    if ($ports.Count -eq 0) {
        try {
            $ports += [System.IO.Ports.SerialPort]::GetPortNames() |
                Sort-Object |
                ForEach-Object {
                    [pscustomobject]@{
                        DeviceID = $_
                        Name = $_
                        Description = ""
                        PNPDeviceID = ""
                    }
                }
        } catch {
        }
    }

    return $ports | Sort-Object DeviceID -Unique
}

function Read-SerialAvailable {
    param([System.IO.Ports.SerialPort]$SerialPort)

    Start-Sleep -Milliseconds 200
    $lines = New-Object System.Collections.Generic.List[string]
    $deadline = [DateTime]::UtcNow.AddMilliseconds(800)
    while ([DateTime]::UtcNow -lt $deadline) {
        if ($SerialPort.BytesToRead -le 0) {
            Start-Sleep -Milliseconds 50
            continue
        }

        try {
            $line = $SerialPort.ReadLine()
            if (-not [string]::IsNullOrWhiteSpace($line)) {
                $lines.Add($line.Trim())
            }
        } catch [System.TimeoutException] {
            break
        }
    }

    return $lines
}

function Send-RelayCommand {
    param(
        [System.IO.Ports.SerialPort]$SerialPort,
        [string]$Command
    )

    $line = $Command.TrimEnd() + "`r`n"
    $bytes = [System.Text.Encoding]::ASCII.GetBytes($line)
    $SerialPort.Write($bytes, 0, $bytes.Length)
    return Read-SerialAvailable -SerialPort $SerialPort
}

if ($ListPorts -or [string]::IsNullOrWhiteSpace($Port)) {
    $candidates = Get-SerialPortCandidates
    if ($candidates.Count -eq 0) {
        Write-Host "No serial ports found."
    } else {
        $candidates | Format-Table DeviceID, Name, Description, PNPDeviceID -AutoSize
    }

    if ($ListPorts) {
        return
    }

    throw "Specify -Port COMx."
}

if ($OffMs -lt 100) {
    throw "-OffMs must be at least 100 ms."
}

$serial = [System.IO.Ports.SerialPort]::new($Port, $Baud, "None", 8, "One")
$serial.ReadTimeout = 500
$serial.WriteTimeout = 1000
$serial.NewLine = "`n"
$serial.DtrEnable = [bool]$AssertDtrRts
$serial.RtsEnable = [bool]$AssertDtrRts

try {
    $serial.Open()
    Start-Sleep -Milliseconds 300

    switch ($Action) {
        "On" {
            $responses = Send-RelayCommand -SerialPort $serial -Command "cfg relay on"
            if (-not $Quiet) { Write-Host "USB power ON" }
        }
        "Off" {
            $responses = Send-RelayCommand -SerialPort $serial -Command "cfg relay off"
            if (-not $Quiet) { Write-Host "USB power OFF" }
        }
        "Status" {
            $responses = Send-RelayCommand -SerialPort $serial -Command "cfg relay status"
        }
        "Cycle" {
            if (-not $Quiet) { Write-Host "USB power OFF for $OffMs ms ..." }
            $responses = Send-RelayCommand -SerialPort $serial -Command "cfg relay off"
            Start-Sleep -Milliseconds $OffMs
            if (-not $Quiet) { Write-Host "USB power ON" }
            $responses += Send-RelayCommand -SerialPort $serial -Command "cfg relay on"
        }
    }

    if (-not $Quiet -and $responses.Count -gt 0) {
        $responses | ForEach-Object { Write-Host $_ }
    }
} finally {
    if ($serial.IsOpen) {
        $serial.Close()
    }
    $serial.Dispose()
}
