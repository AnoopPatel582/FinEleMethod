param(
    [Parameter(Mandatory = $true)]
    [string]$StageDirectory,

    [Parameter(Mandatory = $true)]
    [string]$OutputFile
)

$ErrorActionPreference = "Stop"

$stage = (Resolve-Path -LiteralPath $StageDirectory).Path
$archive = [System.IO.Path]::GetFullPath($OutputFile)
if ([System.IO.Path]::GetExtension($archive) -ne ".zip") {
    throw "Windows application archive must use the .zip extension: $archive"
}

$outputDirectory = Split-Path -Parent $archive
[System.IO.Directory]::CreateDirectory($outputDirectory) | Out-Null
if (Test-Path -LiteralPath $archive) {
    Remove-Item -LiteralPath $archive -Force
}

Compress-Archive -Path (Join-Path $stage "*") -DestinationPath $archive -CompressionLevel Optimal
if (-not (Test-Path -LiteralPath $archive -PathType Leaf)) {
    throw "Windows application archive was not created: $archive"
}

$archiveFile = Get-Item -LiteralPath $archive
$checksumFile = "$archive.sha256"
$checksum = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content -LiteralPath $checksumFile -Value "$checksum  $($archiveFile.Name)" -Encoding ascii
Write-Host "Created Windows application archive: $($archiveFile.FullName) ($($archiveFile.Length) bytes)"
Write-Host "Created SHA-256 checksum: $checksumFile"
