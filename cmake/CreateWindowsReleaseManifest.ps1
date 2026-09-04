param(
    [Parameter(Mandatory = $true)] [string]$ArchiveFile,
    [Parameter(Mandatory = $true)] [string]$ChecksumFile,
    [Parameter(Mandatory = $true)] [string]$SourceCommit,
    [Parameter(Mandatory = $true)] [string]$WorkflowUrl,
    [Parameter(Mandatory = $true)] [string]$OutputFile
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$archive = Get-Item -LiteralPath $ArchiveFile -ErrorAction Stop
$checksum = (Get-Content -LiteralPath $ChecksumFile -Raw).Trim()
if ($checksum -notmatch "^([0-9a-fA-F]{64})  (.+)$") {
    throw "Invalid SHA-256 checksum record: $ChecksumFile"
}
if ($Matches[2] -ne $archive.Name) {
    throw "Checksum filename '$($Matches[2])' does not match archive '$($archive.Name)'."
}
$expectedHash = $Matches[1].ToLowerInvariant()
$actualHash = (Get-FileHash -LiteralPath $archive.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
if ($actualHash -ne $expectedHash) {
    throw "SHA-256 checksum does not match archive: $($archive.FullName)"
}
if ($SourceCommit -notmatch "^[0-9a-fA-F]{40}$") {
    throw "Source commit must be a full 40-character Git commit hash."
}
$workflowUri = $null
if (-not [System.Uri]::TryCreate($WorkflowUrl, [System.UriKind]::Absolute, [ref]$workflowUri) -or
    $workflowUri.Scheme -ne "https" -or $workflowUri.Host -ne "github.com") {
    throw "Workflow URL must be an HTTPS github.com URL."
}

$output = [System.IO.Path]::GetFullPath($OutputFile)
if (Test-Path -LiteralPath $output) {
    throw "Refusing to overwrite existing release manifest: $output"
}
[System.IO.Directory]::CreateDirectory((Split-Path -Parent $output)) | Out-Null
$manifest = [ordered]@{
    schemaVersion = 1
    product = "FinEleMethod"
    platform = "windows-x64"
    sourceCommit = $SourceCommit.ToLowerInvariant()
    workflowRun = $WorkflowUrl
    archive = [ordered]@{
        fileName = $archive.Name
        sizeBytes = $archive.Length
        sha256 = $actualHash
    }
    validation = [ordered]@{
        stagedApplication = "passed"
        extractedArchive = "passed"
    }
}

$json = $manifest | ConvertTo-Json -Depth 4
[System.IO.File]::WriteAllText($output, "$json`n", [System.Text.UTF8Encoding]::new($false))
Write-Host "Created Windows release manifest: $output"
