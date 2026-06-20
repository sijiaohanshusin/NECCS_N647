param(
    [string]$SourceRoot = "D:\Project\NECCS",
    [string]$OutputRoot = "D:\Project\NECCS\N647_BaseBoard\firmware\NECCS_N647\docs\ocrkb",
    [string]$OcrKb = "C:\Users\xiazhiyuan\.local\bin\ocrkb.ps1",
    [string]$PythonExe = "D:\Project\OCR\.venv\Scripts\python.exe",
    [int]$MinimumDocumentTimeoutMinutes = 20,
    [int]$MaximumDocumentTimeoutMinutes = 720
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$sourceRootPath = [IO.Path]::GetFullPath($SourceRoot).TrimEnd("\")
$outputRootPath = [IO.Path]::GetFullPath($OutputRoot).TrimEnd("\")
$ocrRoot = "D:\Project\OCR"
$documentsDir = Join-Path $outputRootPath "documents"
$logsDir = Join-Path $outputRootPath "logs"
$stateDir = Join-Path $logsDir "state"
$reportsDir = Join-Path $outputRootPath "reports"
$catalogCsv = Join-Path $outputRootPath "source_catalog.csv"
$uniqueJson = Join-Path $outputRootPath "unique_documents.json"
$runStatePath = Join-Path $outputRootPath "run_state.json"
$indexPath = Join-Path $outputRootPath "INDEX.md"
$qualityJsonPath = Join-Path $reportsDir "quality_report.json"
$qualityMdPath = Join-Path $reportsDir "quality_report.md"

$excludedDirectoryNames = @(
    ".git",
    "Debug",
    "Release",
    "build",
    "Binary",
    "tmp",
    "OCR_Knowledge",
    "ocrkb",
    "node_modules"
)

foreach ($dir in @($outputRootPath, $documentsDir, $logsDir, $stateDir, $reportsDir)) {
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
}

function Write-Utf8NoBom {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Text
    )

    $tempPath = "$Path.tmp"
    [IO.File]::WriteAllText($tempPath, $Text, [Text.UTF8Encoding]::new($false))
    Move-Item -LiteralPath $tempPath -Destination $Path -Force
}

function Write-JsonFile {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)]$Value,
        [int]$Depth = 12
    )

    Write-Utf8NoBom -Path $Path -Text ($Value | ConvertTo-Json -Depth $Depth)
}

function Get-SafeSlug {
    param([Parameter(Mandatory = $true)][string]$Name)

    $slug = $Name.ToLowerInvariant()
    $slug = [regex]::Replace($slug, "[^a-z0-9\u4e00-\u9fff]+", "-")
    $slug = $slug.Trim("-")
    if ([string]::IsNullOrWhiteSpace($slug)) {
        $slug = "document"
    }
    if ($slug.Length -gt 70) {
        $slug = $slug.Substring(0, 70).TrimEnd("-")
    }
    return $slug
}

function Get-RelativePathCompat {
    param(
        [Parameter(Mandatory = $true)][string]$BasePath,
        [Parameter(Mandatory = $true)][string]$ChildPath
    )

    $baseFull = [IO.Path]::GetFullPath($BasePath).TrimEnd("\")
    $childFull = [IO.Path]::GetFullPath($ChildPath)
    $prefix = "$baseFull\"
    if ($childFull.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) {
        return $childFull.Substring($prefix.Length)
    }
    if ($childFull.Equals($baseFull, [StringComparison]::OrdinalIgnoreCase)) {
        return "."
    }
    throw "Path is outside base path: base=$baseFull child=$childFull"
}

function Convert-JsonOutput {
    param([Parameter(Mandatory = $true)][string]$Text)

    try {
        return $Text | ConvertFrom-Json
    }
    catch {
        $first = $Text.IndexOf("{")
        $last = $Text.LastIndexOf("}")
        if ($first -ge 0 -and $last -gt $first) {
            return $Text.Substring($first, $last - $first + 1) | ConvertFrom-Json
        }
        throw
    }
}

function Quote-ProcessArgument {
    param([Parameter(Mandatory = $true)][string]$Value)

    return '"' + $Value.Replace('"', '\"') + '"'
}

function Stop-ProcessTree {
    param([Parameter(Mandatory = $true)][int]$RootProcessId)

    $allProcesses = @(Get-CimInstance Win32_Process -ErrorAction SilentlyContinue)
    $processIds = New-Object System.Collections.Generic.List[int]

    function Add-ChildProcesses {
        param([int]$ParentProcessId)

        foreach ($child in $allProcesses | Where-Object { $_.ParentProcessId -eq $ParentProcessId }) {
            Add-ChildProcesses -ParentProcessId ([int]$child.ProcessId)
            if (-not $processIds.Contains([int]$child.ProcessId)) {
                $processIds.Add([int]$child.ProcessId)
            }
        }
    }

    Add-ChildProcesses -ParentProcessId $RootProcessId
    if (-not $processIds.Contains($RootProcessId)) {
        $processIds.Add($RootProcessId)
    }
    foreach ($processId in $processIds) {
        Stop-Process -Id $processId -Force -ErrorAction SilentlyContinue
    }
}

function Remove-StaleOcrKbLock {
    $lockPath = Join-Path $ocrRoot "data\runtime\ocrkb.write.lock"
    if (-not (Test-Path -LiteralPath $lockPath)) {
        return
    }

    try {
        $metadata = Get-Content -LiteralPath $lockPath -Raw -Encoding UTF8 | ConvertFrom-Json
        $lockPid = [int]$metadata.pid
        if (-not (Get-Process -Id $lockPid -ErrorAction SilentlyContinue)) {
            Remove-Item -LiteralPath $lockPath -Force
        }
    }
    catch {
        Remove-Item -LiteralPath $lockPath -Force
    }
}

function Get-PdfPageCount {
    param([Parameter(Mandatory = $true)][string]$PdfPath)

    if (-not (Test-Path -LiteralPath $PythonExe)) {
        return 0
    }

    $pythonCode = "import fitz, sys; document = fitz.open(sys.argv[1]); print(document.page_count)"
    try {
        $output = & $PythonExe -c $pythonCode $PdfPath 2>$null
        return [int]($output | Select-Object -Last 1)
    }
    catch {
        return 0
    }
}

function Get-DocumentTimeoutMinutes {
    param([Parameter(Mandatory = $true)][int]$PageCount)

    if ($PageCount -le 0) {
        return $MinimumDocumentTimeoutMinutes
    }

    $calculated = [int][Math]::Ceiling(5 + ($PageCount * 0.1))
    return [Math]::Min(
        $MaximumDocumentTimeoutMinutes,
        [Math]::Max($MinimumDocumentTimeoutMinutes, $calculated)
    )
}

function Invoke-OcrKbCommand {
    param(
        [Parameter(Mandatory = $true)][string[]]$Arguments,
        [Parameter(Mandatory = $true)][string]$LogPath,
        [Parameter(Mandatory = $true)][int]$TimeoutMinutes
    )

    $argumentParts = @(
        "-NoProfile",
        "-ExecutionPolicy", "Bypass",
        "-File", (Quote-ProcessArgument -Value $OcrKb)
    )
    foreach ($argument in $Arguments) {
        $argumentParts += Quote-ProcessArgument -Value $argument
    }

    $startInfo = New-Object System.Diagnostics.ProcessStartInfo
    $startInfo.FileName = "powershell.exe"
    $startInfo.Arguments = $argumentParts -join " "
    $startInfo.WorkingDirectory = $sourceRootPath
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    $startInfo.StandardOutputEncoding = New-Object System.Text.UTF8Encoding($false)
    $startInfo.StandardErrorEncoding = New-Object System.Text.UTF8Encoding($false)

    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $startInfo
    if (-not $process.Start()) {
        throw "Failed to start OCRKB process."
    }
    $stdoutTask = $process.StandardOutput.ReadToEndAsync()
    $stderrTask = $process.StandardError.ReadToEndAsync()

    $completed = $process.WaitForExit($TimeoutMinutes * 60 * 1000)
    $timedOut = -not $completed
    if ($timedOut) {
        Stop-ProcessTree -RootProcessId $process.Id
        Start-Sleep -Seconds 2
        Remove-StaleOcrKbLock
        [void]$process.WaitForExit(5000)
    }
    else {
        $process.WaitForExit()
    }

    $stdout = $stdoutTask.Result
    $stderr = $stderrTask.Result
    $text = @($stdout, $stderr) -join [Environment]::NewLine
    Write-Utf8NoBom -Path $LogPath -Text $text

    $json = $null
    $parseError = ""
    if (-not $timedOut) {
        try {
            $json = Convert-JsonOutput -Text $stdout
        }
        catch {
            $parseError = $_.Exception.Message
        }
    }

    $exitCode = if ($timedOut) { 124 } else { $process.ExitCode }
    $process.Dispose()

    return [pscustomobject]@{
        ExitCode = $exitCode
        TimedOut = $timedOut
        Json = $json
        ParseError = $parseError
        Text = $text
    }
}

function Get-AreaRank {
    param([Parameter(Mandatory = $true)][string]$RelativePath)

    if ($RelativePath -like "Shared_References\*") { return 0 }
    if ($RelativePath -like "N647_BaseBoard\*") { return 1 }
    if ($RelativePath -like "N657_MainBoard\*") { return 2 }
    if ($RelativePath -like "H7_Original\*") { return 3 }
    return 4
}

function Update-RunState {
    param(
        [Parameter(Mandatory = $true)][int]$Total,
        [Parameter(Mandatory = $true)][string]$RunStatus,
        [string]$CurrentSha1 = "",
        [string]$CurrentPath = ""
    )

    $states = @(
        Get-ChildItem -LiteralPath $stateDir -File -Filter "*.json" -ErrorAction SilentlyContinue |
            ForEach-Object {
                try { Get-Content -LiteralPath $_.FullName -Raw -Encoding UTF8 | ConvertFrom-Json } catch { $null }
            } |
            Where-Object { $null -ne $_ }
    )
    $success = @($states | Where-Object { $_.status -eq "success" }).Count
    $failed = @($states | Where-Object { $_.status -eq "failed" }).Count
    $processing = @($states | Where-Object { $_.status -eq "processing" }).Count

    $state = [ordered]@{
        status = $RunStatus
        updated_at = (Get-Date).ToString("o")
        source_root = $sourceRootPath
        output_root = $outputRootPath
        total_unique_documents = $Total
        completed = $success
        failed = $failed
        processing = $processing
        pending = [Math]::Max(0, $Total - $success - $failed - $processing)
        current_sha1 = $CurrentSha1
        current_path = $CurrentPath
        state_directory = $stateDir
    }
    Write-JsonFile -Path $runStatePath -Value $state
}

function Build-QualityReport {
    $manifestFiles = @(
        Get-ChildItem -LiteralPath (Join-Path $ocrRoot "data\ocr_out") -Recurse -File -Filter "manifest.json" -ErrorAction SilentlyContinue
    )
    $documents = @()
    foreach ($manifestFile in $manifestFiles) {
        try {
            $manifest = Get-Content -LiteralPath $manifestFile.FullName -Raw -Encoding UTF8 | ConvertFrom-Json
            $pages = @($manifest.pages)
            $needsReview = @($pages | Where-Object { $_.needs_review }).Count
            $ocrErrors = @($pages | Where-Object { -not [string]::IsNullOrWhiteSpace([string]$_.ocr_error_type) }).Count
            $validationWarnings = @($pages | Where-Object { $_.validation_status -ne "ok" }).Count
            $documents += [pscustomobject]@{
                doc_id = [string]$manifest.doc_id
                source_pdf = [string]$manifest.source_pdf
                status = [string]$manifest.status
                page_count = [int]$manifest.page_count
                error_count = [int]$manifest.error_count
                needs_review_pages = $needsReview
                ocr_error_pages = $ocrErrors
                validation_warning_pages = $validationWarnings
                native_text_pages = @($pages | Where-Object { $_.processing_route -eq "native_text" }).Count
                visual_summary_pages = @($pages | Where-Object { $_.processing_route -eq "visual_summary" }).Count
                ocr_pages = @($pages | Where-Object { $_.processing_route -match "ocr" }).Count
                manifest_path = $manifestFile.FullName
            }
        }
        catch {
            $documents += [pscustomobject]@{
                doc_id = ""
                source_pdf = ""
                status = "manifest_parse_failed"
                page_count = 0
                error_count = 1
                needs_review_pages = 0
                ocr_error_pages = 0
                validation_warning_pages = 0
                native_text_pages = 0
                visual_summary_pages = 0
                ocr_pages = 0
                manifest_path = $manifestFile.FullName
            }
        }
    }

    $summary = [ordered]@{
        generated_at = (Get-Date).ToString("o")
        document_count = $documents.Count
        success_documents = @($documents | Where-Object { $_.status -eq "success" }).Count
        non_success_documents = @($documents | Where-Object { $_.status -ne "success" }).Count
        total_pages = ($documents | Measure-Object page_count -Sum).Sum
        total_page_errors = ($documents | Measure-Object error_count -Sum).Sum
        total_needs_review_pages = ($documents | Measure-Object needs_review_pages -Sum).Sum
        total_ocr_error_pages = ($documents | Measure-Object ocr_error_pages -Sum).Sum
        total_validation_warning_pages = ($documents | Measure-Object validation_warning_pages -Sum).Sum
        documents = $documents
    }
    Write-JsonFile -Path $qualityJsonPath -Value $summary

    $lines = @(
        "# OCRKB quality report",
        "",
        "- Generated: $($summary.generated_at)",
        "- Documents: $($summary.document_count)",
        "- Successful documents: $($summary.success_documents)",
        "- Non-success documents: $($summary.non_success_documents)",
        "- Total pages: $($summary.total_pages)",
        "- Page errors: $($summary.total_page_errors)",
        "- Pages requiring review: $($summary.total_needs_review_pages)",
        "- Pages with OCR errors: $($summary.total_ocr_error_pages)",
        "- Pages with validation warnings: $($summary.total_validation_warning_pages)",
        "",
        "## Documents requiring attention",
        ""
    )
    $attention = @(
        $documents | Where-Object {
            $_.status -ne "success" -or
            $_.error_count -gt 0 -or
            $_.needs_review_pages -gt 0 -or
            $_.ocr_error_pages -gt 0 -or
            $_.validation_warning_pages -gt 0
        } | Sort-Object source_pdf
    )
    if ($attention.Count -eq 0) {
        $lines += "- None."
    }
    else {
        foreach ($doc in $attention) {
            $lines += "- ``$($doc.doc_id)``: status=$($doc.status), pages=$($doc.page_count), errors=$($doc.error_count), review=$($doc.needs_review_pages), ocr_errors=$($doc.ocr_error_pages), validation_warnings=$($doc.validation_warning_pages)"
        }
    }
    Write-Utf8NoBom -Path $qualityMdPath -Text ($lines -join [Environment]::NewLine)
}

function Build-Index {
    param([Parameter(Mandatory = $true)]$UniqueDocuments)

    $lines = @(
        "# NECCS OCR Knowledge Index",
        "",
        "This directory contains freshly generated OCRKB exports. Source PDFs remain in their original locations.",
        "",
        "- Unique documents: $($UniqueDocuments.Count)",
        "- Source catalog: [source_catalog.csv](source_catalog.csv)",
        "- Machine-readable source map: [unique_documents.json](unique_documents.json)",
        "- Run status: [run_state.json](run_state.json)",
        "- Quality report: [reports/quality_report.md](reports/quality_report.md)",
        "",
        "## Documents",
        ""
    )

    foreach ($doc in ($UniqueDocuments | Sort-Object Area, CanonicalRelativePath)) {
        $statePath = Join-Path $stateDir "$($doc.SHA1).json"
        $status = $null
        if (Test-Path -LiteralPath $statePath) {
            try { $status = Get-Content -LiteralPath $statePath -Raw -Encoding UTF8 | ConvertFrom-Json } catch { $status = $null }
        }
        $label = [IO.Path]::GetFileNameWithoutExtension($doc.CanonicalPath)
        if ($null -ne $status -and $status.status -eq "success" -and (Test-Path -LiteralPath $status.output_dir)) {
            $reviewedPath = Join-Path $status.output_dir "reviewed.md"
            $resultPath = Join-Path $status.output_dir "result.md"
            $targetPath = if (Test-Path -LiteralPath $reviewedPath) { $reviewedPath } else { $resultPath }
            $relativeTarget = (Get-RelativePathCompat -BasePath $outputRootPath -ChildPath $targetPath).Replace("\", "/")
            $lines += "- [$label]($relativeTarget) - ``$($doc.CanonicalRelativePath)``"
        }
        else {
            $stateText = if ($null -eq $status) { "pending" } else { [string]$status.status }
            $lines += "- $label - $stateText - ``$($doc.CanonicalRelativePath)``"
        }
    }
    Write-Utf8NoBom -Path $indexPath -Text ($lines -join [Environment]::NewLine)
}

if (-not (Test-Path -LiteralPath $sourceRootPath)) {
    throw "Source root does not exist: $sourceRootPath"
}
if (-not (Test-Path -LiteralPath $OcrKb)) {
    throw "OCRKB command does not exist: $OcrKb"
}
if (-not $outputRootPath.StartsWith($sourceRootPath, [StringComparison]::OrdinalIgnoreCase)) {
    throw "Output root must be inside the source root."
}

$allPdfs = @(
    Get-ChildItem -LiteralPath $sourceRootPath -Recurse -File -Filter "*.pdf" -ErrorAction SilentlyContinue |
        Where-Object {
            $relative = Get-RelativePathCompat -BasePath $sourceRootPath -ChildPath $_.FullName
            $segments = $relative -split "\\"
            -not (@($segments | Where-Object { $excludedDirectoryNames -contains $_ }).Count)
        }
)

$sourceEntries = @()
foreach ($pdf in $allPdfs) {
    $relative = Get-RelativePathCompat -BasePath $sourceRootPath -ChildPath $pdf.FullName
    $sha1 = (Get-FileHash -LiteralPath $pdf.FullName -Algorithm SHA1).Hash.ToLowerInvariant()
    $sourceEntries += [pscustomobject]@{
        SHA1 = $sha1
        FullPath = $pdf.FullName
        RelativePath = $relative
        Area = ($relative -split "\\")[0]
        SizeBytes = $pdf.Length
        SizeMB = [Math]::Round($pdf.Length / 1MB, 3)
        LastWriteTime = $pdf.LastWriteTime.ToString("o")
        AreaRank = Get-AreaRank -RelativePath $relative
    }
}

$uniqueDocuments = @()
$catalogRows = @()
foreach ($group in ($sourceEntries | Group-Object SHA1)) {
    $ordered = @($group.Group | Sort-Object AreaRank, @{ Expression = { $_.RelativePath.Length } }, RelativePath)
    $canonical = $ordered[0]
    $uniqueDocuments += [pscustomobject]@{
        SHA1 = $group.Name
        SHA1Short = $group.Name.Substring(0, 8)
        CanonicalPath = $canonical.FullPath
        CanonicalRelativePath = $canonical.RelativePath
        Area = $canonical.Area
        SizeBytes = $canonical.SizeBytes
        SizeMB = $canonical.SizeMB
        DuplicateCount = $ordered.Count
        PageCount = Get-PdfPageCount -PdfPath $canonical.FullPath
        SourcePaths = @($ordered.FullPath)
        SourceRelativePaths = @($ordered.RelativePath)
    }
    foreach ($entry in $ordered) {
        $catalogRows += [pscustomobject]@{
            SHA1 = $entry.SHA1
            Canonical = ($entry.FullPath -eq $canonical.FullPath)
            Area = $entry.Area
            RelativePath = $entry.RelativePath
            FullPath = $entry.FullPath
            SizeMB = $entry.SizeMB
            LastWriteTime = $entry.LastWriteTime
        }
    }
}

$uniqueDocuments = @($uniqueDocuments | Sort-Object Area, CanonicalRelativePath)
$catalogRows | Sort-Object Area, RelativePath | Export-Csv -LiteralPath $catalogCsv -NoTypeInformation -Encoding UTF8
Write-JsonFile -Path $uniqueJson -Value $uniqueDocuments
Update-RunState -Total $uniqueDocuments.Count -RunStatus "running"
Build-Index -UniqueDocuments $uniqueDocuments

foreach ($doc in $uniqueDocuments) {
    $docStatePath = Join-Path $stateDir "$($doc.SHA1).json"
    if (Test-Path -LiteralPath $docStatePath) {
        try {
            $existingState = Get-Content -LiteralPath $docStatePath -Raw -Encoding UTF8 | ConvertFrom-Json
            if ($existingState.status -eq "success" -and (Test-Path -LiteralPath $existingState.output_dir)) {
                continue
            }
        }
        catch {
            # A malformed state file is overwritten by the retry below.
        }
    }

    $startedAt = Get-Date
    $slug = Get-SafeSlug -Name ([IO.Path]::GetFileNameWithoutExtension($doc.CanonicalPath))
    $logPrefix = "$($doc.SHA1Short)-$slug"
    $ingestLog = Join-Path $logsDir "$logPrefix-ingest.log"
    $exportLog = Join-Path $logsDir "$logPrefix-export.log"
    $areaOutput = Join-Path $documentsDir $doc.Area
    New-Item -ItemType Directory -Path $areaOutput -Force | Out-Null
    $documentTimeoutMinutes = Get-DocumentTimeoutMinutes -PageCount ([int]$doc.PageCount)

    $processingState = [ordered]@{
        sha1 = $doc.SHA1
        status = "processing"
        canonical_path = $doc.CanonicalPath
        canonical_relative_path = $doc.CanonicalRelativePath
        started_at = $startedAt.ToString("o")
        updated_at = $startedAt.ToString("o")
        ingest_log = $ingestLog
        export_log = $exportLog
        doc_id = ""
        output_dir = ""
        page_count = [int]$doc.PageCount
        timeout_minutes = $documentTimeoutMinutes
        error = ""
    }
    Write-JsonFile -Path $docStatePath -Value $processingState
    Update-RunState -Total $uniqueDocuments.Count -RunStatus "running" -CurrentSha1 $doc.SHA1 -CurrentPath $doc.CanonicalPath

    try {
        $ingest = Invoke-OcrKbCommand `
            -Arguments @("ingest", $doc.CanonicalPath) `
            -LogPath $ingestLog `
            -TimeoutMinutes $documentTimeoutMinutes
        if ($ingest.ExitCode -ne 0 -or $null -eq $ingest.Json) {
            if ($ingest.TimedOut) {
                throw "Ingest exceeded the document timeout of $documentTimeoutMinutes minutes."
            }
            throw "Ingest failed with exit code $($ingest.ExitCode). JSON parse error: $($ingest.ParseError)"
        }
        $ingestResult = @($ingest.Json.results)[0]
        if ($null -eq $ingestResult -or [string]::IsNullOrWhiteSpace([string]$ingestResult.doc_id)) {
            throw "Ingest did not return a document ID."
        }

        $export = Invoke-OcrKbCommand `
            -Arguments @("export", $doc.CanonicalPath, "--out", $areaOutput) `
            -LogPath $exportLog `
            -TimeoutMinutes 30
        if ($export.ExitCode -ne 0 -or $null -eq $export.Json) {
            throw "Export failed with exit code $($export.ExitCode). JSON parse error: $($export.ParseError)"
        }
        $exportResult = @($export.Json.exports)[0]
        if ($null -eq $exportResult -or -not (Test-Path -LiteralPath $exportResult.output_dir)) {
            throw "Export did not produce a readable output directory."
        }

        $completedAt = Get-Date
        $successState = [ordered]@{
            sha1 = $doc.SHA1
            status = "success"
            canonical_path = $doc.CanonicalPath
            canonical_relative_path = $doc.CanonicalRelativePath
            started_at = $startedAt.ToString("o")
            completed_at = $completedAt.ToString("o")
            elapsed_seconds = [Math]::Round(($completedAt - $startedAt).TotalSeconds, 2)
            ingest_log = $ingestLog
            export_log = $exportLog
            doc_id = [string]$ingestResult.doc_id
            ingest_status = [string]$ingestResult.status
            output_dir = [string]$exportResult.output_dir
            page_count = [int]$doc.PageCount
            timeout_minutes = $documentTimeoutMinutes
            error = ""
        }
        Write-JsonFile -Path $docStatePath -Value $successState
    }
    catch {
        $failedAt = Get-Date
        $failedState = [ordered]@{
            sha1 = $doc.SHA1
            status = "failed"
            canonical_path = $doc.CanonicalPath
            canonical_relative_path = $doc.CanonicalRelativePath
            started_at = $startedAt.ToString("o")
            failed_at = $failedAt.ToString("o")
            elapsed_seconds = [Math]::Round(($failedAt - $startedAt).TotalSeconds, 2)
            ingest_log = $ingestLog
            export_log = $exportLog
            doc_id = ""
            output_dir = ""
            page_count = [int]$doc.PageCount
            timeout_minutes = $documentTimeoutMinutes
            error = $_.Exception.Message
        }
        Write-JsonFile -Path $docStatePath -Value $failedState
    }

    Update-RunState -Total $uniqueDocuments.Count -RunStatus "running"
    Build-QualityReport
    Build-Index -UniqueDocuments $uniqueDocuments
}

$doctorLog = Join-Path $logsDir "final-doctor.log"
$statusLog = Join-Path $logsDir "final-status.log"
$doctor = Invoke-OcrKbCommand -Arguments @("doctor") -LogPath $doctorLog -TimeoutMinutes 5
$status = Invoke-OcrKbCommand -Arguments @("status") -LogPath $statusLog -TimeoutMinutes 5
Build-QualityReport
Build-Index -UniqueDocuments $uniqueDocuments

$failedCount = @(
    Get-ChildItem -LiteralPath $stateDir -File -Filter "*.json" |
        ForEach-Object {
            try { Get-Content -LiteralPath $_.FullName -Raw -Encoding UTF8 | ConvertFrom-Json } catch { $null }
        } |
        Where-Object { $null -ne $_ -and $_.status -eq "failed" }
).Count
$finalStatus = if ($failedCount -eq 0 -and $doctor.ExitCode -eq 0 -and $status.ExitCode -eq 0) {
    "completed"
}
else {
    "completed_with_errors"
}
Update-RunState -Total $uniqueDocuments.Count -RunStatus $finalStatus
