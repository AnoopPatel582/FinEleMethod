$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$fixture = Join-Path ([System.IO.Path]::GetTempPath()) ("FinEleMethod archive tests " + [guid]::NewGuid())
$stage = Join-Path $fixture "stage"
New-Item -ItemType Directory -Path $stage | Out-Null
[System.IO.File]::WriteAllText((Join-Path $stage "payload.txt"), "archive regression fixture")
$builder = Join-Path $PSScriptRoot "CreateWindowsArchive.ps1"
$verifier = Join-Path $PSScriptRoot "VerifyWindowsArchiveChecksum.ps1"

function Assert-Rejected([scriptblock]$Action, [string]$ExpectedMessage) {
    try { & $Action }
    catch {
        if ($_.Exception.Message -notlike $ExpectedMessage) { throw }
        return
    }
    throw "Expected rejection matching: $ExpectedMessage"
}

$archive = Join-Path $fixture "first.zip"
& $builder -StageDirectory $stage -OutputFile $archive
& $verifier -ArchiveFile $archive -ChecksumFile "$archive.sha256"
$extracted = Join-Path $fixture "extracted"
Expand-Archive -LiteralPath $archive -DestinationPath $extracted
if ([System.IO.File]::ReadAllText((Join-Path $extracted "payload.txt")) -ne "archive regression fixture") {
    throw "ZIP round-trip changed the payload."
}

$zipHash = (Get-FileHash -LiteralPath $archive).Hash
$checksumHash = (Get-FileHash -LiteralPath "$archive.sha256").Hash
Assert-Rejected { & $builder -StageDirectory $stage -OutputFile $archive } '*Refusing to overwrite*'
if ((Get-FileHash -LiteralPath $archive).Hash -ne $zipHash -or
    (Get-FileHash -LiteralPath "$archive.sha256").Hash -ne $checksumHash) {
    throw "Rejected rebuild changed an existing package."
}

$reserved = Join-Path $fixture "reserved.zip"
[System.IO.File]::WriteAllText("$reserved.sha256", "retained checksum")
Assert-Rejected { & $builder -StageDirectory $stage -OutputFile $reserved } '*Refusing to overwrite*'
if ((Test-Path -LiteralPath $reserved) -or
    [System.IO.File]::ReadAllText("$reserved.sha256") -ne "retained checksum") {
    throw "Existing checksum was not preserved."
}

$inside = Join-Path $stage "nested\invalid.zip"
Assert-Rejected { & $builder -StageDirectory $stage -OutputFile $inside } '*must be outside*'
if (Test-Path -LiteralPath (Split-Path -Parent $inside)) {
    throw "Rejected in-stage output created a directory."
}
Assert-Rejected { & $builder -StageDirectory $stage -OutputFile (Join-Path $fixture "invalid.txt") } '*must use the .zip extension*'

# Similar prefixes are siblings, not children of the staged directory.
$sibling = Join-Path $fixture "stage-other\valid.zip"
& $builder -StageDirectory $stage -OutputFile $sibling
& $verifier -ArchiveFile $sibling -ChecksumFile "$sibling.sha256"
Write-Host "Windows archive regression checks passed. Retained fixture: $fixture"
