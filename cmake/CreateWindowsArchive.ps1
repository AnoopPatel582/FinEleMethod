param(
    [Parameter(Mandatory = $true)]
    [string]$StageDirectory,

    [Parameter(Mandatory = $true)]
    [string]$OutputFile
)

$ErrorActionPreference = "Stop"

$stage = (Resolve-Path -LiteralPath $StageDirectory).Path
$archive = [System.IO.Path]::GetFullPath($OutputFile)
$checksumFile = "$archive.sha256"
if ([System.IO.Path]::GetExtension($archive) -ne ".zip") {
    throw "Windows application archive must use the .zip extension: $archive"
}

$stagePrefix = $stage.TrimEnd([char[]]@('\', '/')) + [System.IO.Path]::DirectorySeparatorChar
if ($archive.StartsWith($stagePrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Windows archive output must be outside the staged application directory."
}
foreach ($destination in @($archive, $checksumFile)) {
    if (Test-Path -LiteralPath $destination) {
        throw "Refusing to overwrite existing package file: $destination. Choose a new output filename."
    }
}

$outputDirectory = Split-Path -Parent $archive
[System.IO.Directory]::CreateDirectory($outputDirectory) | Out-Null

Compress-Archive -Path (Join-Path $stage "*") -DestinationPath $archive -CompressionLevel Optimal
if (-not (Test-Path -LiteralPath $archive -PathType Leaf)) {
    throw "Windows application archive was not created: $archive"
}

$archiveFile = Get-Item -LiteralPath $archive
$checksum = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
# CreateNew also protects a checksum created by another process after preflight.
$checksumStream = [System.IO.File]::Open($checksumFile, [System.IO.FileMode]::CreateNew)
try {
    $record = [System.Text.Encoding]::ASCII.GetBytes("$checksum  $($archiveFile.Name)`n")
    $checksumStream.Write($record, 0, $record.Length)
}
finally {
    $checksumStream.Dispose()
}
Write-Host "Created Windows application archive: $($archiveFile.FullName) ($($archiveFile.Length) bytes)"
Write-Host "Created SHA-256 checksum: $checksumFile"
