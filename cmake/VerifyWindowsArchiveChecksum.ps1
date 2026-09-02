param(
    [Parameter(Mandatory = $true)]
    [string]$ArchiveFile,

    [Parameter(Mandatory = $true)]
    [string]$ChecksumFile
)

$ErrorActionPreference = "Stop"

$archive = Get-Item -LiteralPath $ArchiveFile -ErrorAction Stop
$checksumPath = (Resolve-Path -LiteralPath $ChecksumFile).Path
$record = (Get-Content -LiteralPath $checksumPath -Raw).Trim()
if ($record -notmatch "^([0-9a-fA-F]{64})  (.+)$") {
    throw "Invalid SHA-256 checksum record: $checksumPath"
}
if ($Matches[2] -ne $archive.Name) {
    throw "Checksum filename '$($Matches[2])' does not match archive '$($archive.Name)'."
}

$expected = $Matches[1].ToLowerInvariant()
$actual = (Get-FileHash -LiteralPath $archive.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
if ($actual -ne $expected) {
    throw "SHA-256 checksum does not match archive: $($archive.FullName)"
}

Write-Host "Verified SHA-256 checksum for $($archive.Name): $actual"
