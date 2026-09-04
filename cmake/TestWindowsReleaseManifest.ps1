$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$fixture = Join-Path ([System.IO.Path]::GetTempPath()) ("FinEleMethod manifest tests " + [guid]::NewGuid())
New-Item -ItemType Directory -Path $fixture | Out-Null
$archive = Join-Path $fixture "FinEleMethod-windows-x64.zip"
[System.IO.File]::WriteAllText($archive, "release manifest fixture")
$hash = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
$checksum = "$archive.sha256"
[System.IO.File]::WriteAllText($checksum, "$hash  FinEleMethod-windows-x64.zip`n")
$creator = Join-Path $PSScriptRoot "CreateWindowsReleaseManifest.ps1"
$commit = "0123456789abcdef0123456789abcdef01234567"
$url = "https://github.com/AnoopPatel582/FinEleMethod/actions/runs/123"

function Assert-Rejected([scriptblock]$Action, [string]$ExpectedMessage) {
    try { & $Action }
    catch {
        if ($_.Exception.Message -notlike $ExpectedMessage) { throw }
        return
    }
    throw "Expected rejection matching: $ExpectedMessage"
}

$manifestFile = Join-Path $fixture "release-manifest.json"
& $creator -ArchiveFile $archive -ChecksumFile $checksum -SourceCommit $commit `
    -WorkflowUrl $url -OutputFile $manifestFile
$manifest = Get-Content -LiteralPath $manifestFile -Raw | ConvertFrom-Json
if ($manifest.schemaVersion -ne 1 -or $manifest.product -ne "FinEleMethod" -or
    $manifest.platform -ne "windows-x64" -or $manifest.sourceCommit -ne $commit -or
    $manifest.workflowRun -ne $url -or $manifest.archive.fileName -ne (Split-Path -Leaf $archive) -or
    $manifest.archive.sizeBytes -ne (Get-Item $archive).Length -or
    $manifest.archive.sha256 -ne $hash -or
    $manifest.validation.stagedApplication -ne "passed" -or
    $manifest.validation.extractedArchive -ne "passed") {
    throw "Release manifest content did not match the verified fixture."
}

Assert-Rejected { & $creator -ArchiveFile $archive -ChecksumFile $checksum -SourceCommit $commit `
    -WorkflowUrl $url -OutputFile $manifestFile } '*Refusing to overwrite*'
Assert-Rejected { & $creator -ArchiveFile $archive -ChecksumFile $checksum -SourceCommit "abc" `
    -WorkflowUrl $url -OutputFile (Join-Path $fixture "bad-commit.json") } '*40-character*'
Assert-Rejected { & $creator -ArchiveFile $archive -ChecksumFile $checksum -SourceCommit $commit `
    -WorkflowUrl "http://example.com/run" -OutputFile (Join-Path $fixture "bad-url.json") } '*HTTPS github.com*'

[System.IO.File]::WriteAllText($archive, "changed")
Assert-Rejected { & $creator -ArchiveFile $archive -ChecksumFile $checksum -SourceCommit $commit `
    -WorkflowUrl $url -OutputFile (Join-Path $fixture "bad-hash.json") } '*does not match*'
Write-Host "Windows release manifest regression checks passed. Retained fixture: $fixture"
