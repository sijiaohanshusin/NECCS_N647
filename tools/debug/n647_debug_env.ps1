[CmdletBinding()]
param(
    [switch]$CheckOnly,
    [switch]$AsJson,
    [string]$CubeIdeRoot = $env:STM32CUBEIDE_ROOT,
    [string]$OpenOcdPath = $env:N647_OPENOCD,
    [string]$OpenOcdScripts = $env:N647_OPENOCD_SCRIPTS,
    [string]$GdbPath = $env:N647_GDB,
    [string]$CubeProgrammerCli = $env:N647_CUBEPROG_CLI,
    [string]$ExternalLoader = $env:N647_EXT_LOADER
)

$ErrorActionPreference = "Stop"

function Resolve-N647RepoRoot {
    $root = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
    return $root.TrimEnd('\', '/')
}

function Resolve-N647CubeIdeRoot {
    param([string]$RequestedRoot)

    if (-not [string]::IsNullOrWhiteSpace($RequestedRoot)) {
        $candidate = [System.IO.Path]::GetFullPath($RequestedRoot)
        if (Test-Path -LiteralPath (Join-Path $candidate "headless-build.bat") -PathType Leaf) {
            return $candidate.TrimEnd('\', '/')
        }
    }

    $install = Get-ChildItem -LiteralPath "C:\ST" -Directory -Filter "STM32CubeIDE_*" -ErrorAction SilentlyContinue |
        Sort-Object Name -Descending |
        Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName "STM32CubeIDE\headless-build.bat") -PathType Leaf } |
        Select-Object -First 1

    if ($null -ne $install) {
        return (Join-Path $install.FullName "STM32CubeIDE")
    }

    return $null
}

function Resolve-N647FirstExistingFile {
    param([string[]]$Candidates)

    foreach ($candidate in $Candidates) {
        if ([string]::IsNullOrWhiteSpace($candidate)) {
            continue
        }

        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return [System.IO.Path]::GetFullPath($candidate)
        }
    }

    return $null
}

function Resolve-N647ToolFromCubeIdePlugin {
    param(
        [string]$CubeIdeRoot,
        [string]$PluginPattern,
        [string]$RelativeToolPath
    )

    if ([string]::IsNullOrWhiteSpace($CubeIdeRoot)) {
        return $null
    }

    $pluginRoot = Join-Path $CubeIdeRoot "plugins"
    $plugins = Get-ChildItem -Path $pluginRoot -Directory -Filter $PluginPattern -ErrorAction SilentlyContinue |
        Sort-Object Name -Descending

    foreach ($plugin in $plugins) {
        $candidate = Join-Path $plugin.FullName $RelativeToolPath
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return [System.IO.Path]::GetFullPath($candidate)
        }
    }

    return $null
}

function Resolve-N647OpenOcdScripts {
    param(
        [string]$CubeIdeRoot,
        [string]$RequestedScripts
    )

    if (-not [string]::IsNullOrWhiteSpace($RequestedScripts)) {
        $target = Join-Path $RequestedScripts "target\stm32n6x.cfg"
        $interface = Join-Path $RequestedScripts "interface\stlink-dap.cfg"
        if ((Test-Path -LiteralPath $target -PathType Leaf) -and
            (Test-Path -LiteralPath $interface -PathType Leaf)) {
            return [System.IO.Path]::GetFullPath($RequestedScripts).TrimEnd('\', '/')
        }
    }

    if (-not [string]::IsNullOrWhiteSpace($CubeIdeRoot)) {
        $pluginRoot = Join-Path $CubeIdeRoot "plugins"
        $stm32n6Cfg = Get-ChildItem -Path $pluginRoot -Recurse -ErrorAction SilentlyContinue -Filter "stm32n6x.cfg" |
            Where-Object { $_.FullName -like "*\resources\openocd\st_scripts\target\stm32n6x.cfg" } |
            Sort-Object FullName -Descending |
            Select-Object -First 1

        if ($null -ne $stm32n6Cfg) {
            return [System.IO.Path]::GetFullPath((Join-Path $stm32n6Cfg.DirectoryName "..")).TrimEnd('\', '/')
        }
    }

    return $null
}

function Resolve-N647CommandPath {
    param([string]$Name)

    $found = where.exe $Name 2>$null | Select-Object -First 1
    if ([string]::IsNullOrWhiteSpace($found)) {
        return $null
    }

    return [System.IO.Path]::GetFullPath($found)
}

function Get-N647ToolVersionLine {
    param(
        [string]$FilePath,
        [string[]]$Arguments
    )

    if ([string]::IsNullOrWhiteSpace($FilePath) -or
        -not (Test-Path -LiteralPath $FilePath -PathType Leaf)) {
        return $null
    }

    $oldErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        $lines = & $FilePath @Arguments 2>&1
        return ($lines | ForEach-Object { "$_" } | Where-Object { -not [string]::IsNullOrWhiteSpace($_.Trim()) } | Select-Object -First 1)
    } catch {
        return $_.Exception.Message
    } finally {
        $ErrorActionPreference = $oldErrorActionPreference
    }
}

function Get-N647DebugEnv {
    param(
        [string]$CubeIdeRoot = $env:STM32CUBEIDE_ROOT,
        [string]$OpenOcdPath = $env:N647_OPENOCD,
        [string]$OpenOcdScripts = $env:N647_OPENOCD_SCRIPTS,
        [string]$GdbPath = $env:N647_GDB,
        [string]$CubeProgrammerCli = $env:N647_CUBEPROG_CLI,
        [string]$ExternalLoader = $env:N647_EXT_LOADER
    )

    $repoRoot = Resolve-N647RepoRoot
    $cubeIde = Resolve-N647CubeIdeRoot -RequestedRoot $CubeIdeRoot

    $openOcd = Resolve-N647FirstExistingFile @(
        $OpenOcdPath,
        (Resolve-N647ToolFromCubeIdePlugin -CubeIdeRoot $cubeIde -PluginPattern "com.st.stm32cube.ide.mcu.externaltools.openocd.win32_*" -RelativeToolPath "tools\bin\openocd.exe")
    )

    $scripts = Resolve-N647OpenOcdScripts -CubeIdeRoot $cubeIde -RequestedScripts $OpenOcdScripts

    $gdb = Resolve-N647FirstExistingFile @(
        $GdbPath,
        (Resolve-N647CommandPath "arm-none-eabi-gdb.exe"),
        (Resolve-N647ToolFromCubeIdePlugin -CubeIdeRoot $cubeIde -PluginPattern "com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*" -RelativeToolPath "tools\bin\arm-none-eabi-gdb.exe")
    )

    $cubeProgrammer = Resolve-N647FirstExistingFile @(
        $CubeProgrammerCli,
        "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe",
        (Resolve-N647ToolFromCubeIdePlugin -CubeIdeRoot $cubeIde -PluginPattern "com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.win32_*" -RelativeToolPath "tools\bin\STM32_Programmer_CLI.exe")
    )

    $loader = Resolve-N647FirstExistingFile @(
        $ExternalLoader,
        "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\ExternalLoader\MX25UM25645G_ATK-CNN647B_ExtMemLoader.stldr",
        (Resolve-N647ToolFromCubeIdePlugin -CubeIdeRoot $cubeIde -PluginPattern "com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.win32_*" -RelativeToolPath "tools\bin\ExternalLoader\MX25UM25645G_ATK-CNN647B_ExtMemLoader.stldr")
    )

    $stlinkGdb = Resolve-N647FirstExistingFile @(
        (Resolve-N647ToolFromCubeIdePlugin -CubeIdeRoot $cubeIde -PluginPattern "com.st.stm32cube.ide.mcu.externaltools.stlink-gdb-server.win32_*" -RelativeToolPath "tools\bin\ST-LINK_gdbserver.exe")
    )

    $jlinkGdb = Resolve-N647FirstExistingFile @(
        "C:\Program Files\SEGGER\JLink_V946\JLinkGDBServerCL.exe",
        (Resolve-N647ToolFromCubeIdePlugin -CubeIdeRoot $cubeIde -PluginPattern "com.st.stm32cube.ide.mcu.externaltools.jlink.win32_*" -RelativeToolPath "tools\bin\JLinkGDBServerCL.exe")
    )

    $debugElf = Join-Path $repoRoot "NECCS_N647_App\STM32CubeIDE\Appli\Debug\NECCS_N647_App_Appli.elf"
    $releaseBundle = Join-Path $repoRoot "_flash_images\n647_boot_bundle.hex"
    $logRoot = Join-Path $repoRoot "_debug_logs"
    $openOcdRamCfg = Join-Path $repoRoot "tools\debug\n647_openocd_ram.cfg"

    $problems = New-Object System.Collections.Generic.List[string]
    if ([string]::IsNullOrWhiteSpace($cubeIde)) { $problems.Add("STM32CubeIDE root not found.") }
    if ([string]::IsNullOrWhiteSpace($openOcd)) { $problems.Add("ST OpenOCD not found.") }
    if ([string]::IsNullOrWhiteSpace($scripts)) { $problems.Add("ST OpenOCD stm32n6 scripts not found.") }
    if ([string]::IsNullOrWhiteSpace($gdb)) { $problems.Add("arm-none-eabi-gdb not found.") }
    if ([string]::IsNullOrWhiteSpace($cubeProgrammer)) { $problems.Add("STM32_Programmer_CLI not found.") }
    if ([string]::IsNullOrWhiteSpace($loader)) { $problems.Add("Official N647 external loader not found.") }

    [pscustomobject]@{
        RepoRoot = $repoRoot
        CubeIdeRoot = $cubeIde
        OpenOcdExe = $openOcd
        OpenOcdScripts = $scripts
        GdbExe = $gdb
        CubeProgrammerCli = $cubeProgrammer
        ExternalLoader = $loader
        StlinkGdbServer = $stlinkGdb
        JlinkGdbServer = $jlinkGdb
        DebugElf = $debugElf
        ReleaseBundleHex = $releaseBundle
        OpenOcdRamCfg = $openOcdRamCfg
        LogRoot = $logRoot
        OpenOcdVersion = Get-N647ToolVersionLine -FilePath $openOcd -Arguments @("--version")
        GdbVersion = Get-N647ToolVersionLine -FilePath $gdb -Arguments @("--version")
        CubeProgrammerVersion = Get-N647ToolVersionLine -FilePath $cubeProgrammer -Arguments @("--version")
        Problems = @($problems)
    }
}

function Write-N647DebugEnv {
    param([object]$EnvInfo)

    Write-Host "N647 debug environment"
    Write-Host "RepoRoot             : $($EnvInfo.RepoRoot)"
    Write-Host "CubeIdeRoot          : $($EnvInfo.CubeIdeRoot)"
    Write-Host "OpenOcdExe           : $($EnvInfo.OpenOcdExe)"
    Write-Host "OpenOcdScripts       : $($EnvInfo.OpenOcdScripts)"
    Write-Host "GdbExe               : $($EnvInfo.GdbExe)"
    Write-Host "CubeProgrammerCli    : $($EnvInfo.CubeProgrammerCli)"
    Write-Host "ExternalLoader       : $($EnvInfo.ExternalLoader)"
    Write-Host "StlinkGdbServer      : $($EnvInfo.StlinkGdbServer)"
    Write-Host "JlinkGdbServer       : $($EnvInfo.JlinkGdbServer)"
    Write-Host "DebugElf             : $($EnvInfo.DebugElf)"
    Write-Host "ReleaseBundleHex     : $($EnvInfo.ReleaseBundleHex)"
    Write-Host "OpenOcdRamCfg        : $($EnvInfo.OpenOcdRamCfg)"
    Write-Host "LogRoot              : $($EnvInfo.LogRoot)"
    Write-Host "OpenOCD version      : $($EnvInfo.OpenOcdVersion)"
    Write-Host "GDB version          : $($EnvInfo.GdbVersion)"
    Write-Host "CubeProgrammer       : $($EnvInfo.CubeProgrammerVersion)"

    if ($EnvInfo.Problems.Count -gt 0) {
        Write-Host ""
        Write-Host "Problems:"
        foreach ($problem in $EnvInfo.Problems) {
            Write-Host "  - $problem"
        }
    }
}

if ($MyInvocation.InvocationName -ne ".") {
    $envInfo = Get-N647DebugEnv `
        -CubeIdeRoot $CubeIdeRoot `
        -OpenOcdPath $OpenOcdPath `
        -OpenOcdScripts $OpenOcdScripts `
        -GdbPath $GdbPath `
        -CubeProgrammerCli $CubeProgrammerCli `
        -ExternalLoader $ExternalLoader

    if ($AsJson) {
        $envInfo | ConvertTo-Json -Depth 4
    } else {
        Write-N647DebugEnv -EnvInfo $envInfo
    }

    if ($CheckOnly -and $envInfo.Problems.Count -gt 0) {
        throw "N647 debug environment check failed."
    }
}
