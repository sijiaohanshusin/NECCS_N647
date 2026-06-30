Param(
    [switch]$CheckOnly
)

$ErrorActionPreference = 'Stop'

. "$PSScriptRoot\n647_touchgfx_common.ps1"

$repoRoot = Get-N647RepoRoot
$touchgfxDir = Get-N647TouchGFXProjectDir
$projectFile = Join-Path $repoRoot 'NECCS_N647_App\STM32CubeIDE\Appli\.project'

if (-not (Test-Path -LiteralPath $projectFile)) {
    throw "CubeIDE .project not found: $projectFile"
}

$generatedSrcDirs = @(
    'generated\fonts\src',
    'generated\images\src',
    'generated\texts\src',
    'generated\videos\src'
)

$desiredLinks = [ordered]@{}
foreach ($relativeDir in $generatedSrcDirs) {
    $dir = Join-Path $touchgfxDir $relativeDir
    if (-not (Test-Path -LiteralPath $dir)) {
        continue
    }

    Get-ChildItem -LiteralPath $dir -Filter '*.cpp' -File | Sort-Object Name | ForEach-Object {
        $generatedRelative = ('Appli/TouchGFX/' + (($relativeDir -replace '\\', '/') + '/' + $_.Name))
        $location = 'PARENT-2-PROJECT_LOC/' + $generatedRelative
        $name = 'Application/User/generated/' + $_.Name
        if ($desiredLinks.Contains($name) -and $desiredLinks[$name] -ne $location) {
            $subdir = ($relativeDir -replace '^generated\\', '') -replace '\\src$', ''
            $name = 'Application/User/generated/' + $subdir + '/' + $_.Name
        }
        $desiredLinks[$name] = $location
    }
}

if ($desiredLinks.Count -eq 0) {
    throw "No generated TouchGFX source files found under $touchgfxDir"
}

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$xml = [System.IO.File]::ReadAllText($projectFile)
$linkPattern = '(?s)\r?\n\t\t<link>\r?\n\t\t\t<name>([^<]+)</name>\r?\n\t\t\t<type>1</type>\r?\n\t\t\t<locationURI>([^<]+)</locationURI>\r?\n\t\t</link>'
$matches = [regex]::Matches($xml, $linkPattern)
$existingByName = @{}
$removeBlocks = New-Object System.Collections.Generic.List[string]

foreach ($match in $matches) {
    $name = [System.Net.WebUtility]::HtmlDecode($match.Groups[1].Value)
    $location = [System.Net.WebUtility]::HtmlDecode($match.Groups[2].Value)
    $existingByName[$name] = $location

    $isGeneratedSource = $name -like 'Application/User/generated/*' -and
        $location -match '^PARENT-2-PROJECT_LOC/Appli/TouchGFX/generated/(fonts|images|texts|videos)/src/[^/]+\.cpp$'

    if ($isGeneratedSource) {
        $actualPath = Join-Path $repoRoot ($location -replace '^PARENT-2-PROJECT_LOC/', 'NECCS_N647_App/' -replace '/', '\')
        if (-not (Test-Path -LiteralPath $actualPath)) {
            $removeBlocks.Add($match.Value) | Out-Null
        }
    }
}

$missing = New-Object System.Collections.Generic.List[object]
foreach ($entry in $desiredLinks.GetEnumerator()) {
    if (-not $existingByName.ContainsKey($entry.Key)) {
        $missing.Add([pscustomobject]@{ Name = $entry.Key; Location = $entry.Value }) | Out-Null
    }
}

Write-Host "Generated TouchGFX source files : $($desiredLinks.Count)"
Write-Host "Missing CubeIDE links           : $($missing.Count)"
Write-Host "Stale CubeIDE links             : $($removeBlocks.Count)"

if ($CheckOnly) {
    if ($missing.Count -gt 0) {
        $missing | Select-Object -First 20 | Format-Table -AutoSize
        if ($missing.Count -gt 20) {
            Write-Host "... $($missing.Count - 20) more"
        }
    }
    exit 0
}

foreach ($block in $removeBlocks) {
    $xml = $xml.Replace($block, '')
}

if ($missing.Count -gt 0) {
    $newBlocks = New-Object System.Collections.Generic.List[string]
    foreach ($item in $missing) {
        $newBlocks.Add(
            "`t`t<link>`r`n" +
            "`t`t`t<name>$($item.Name)</name>`r`n" +
            "`t`t`t<type>1</type>`r`n" +
            "`t`t`t<locationURI>$($item.Location)</locationURI>`r`n" +
            "`t`t</link>"
        ) | Out-Null
    }

    $insertText = ($newBlocks -join "`r`n") + "`r`n"
    $marker = "`t</linkedResources>"
    if (-not $xml.Contains($marker)) {
        throw "Could not find linkedResources closing tag in $projectFile"
    }
    $xml = $xml.Replace($marker, $insertText + $marker)
}

[System.IO.File]::WriteAllText($projectFile, $xml, $utf8NoBom)
Write-Host "Updated $projectFile"
