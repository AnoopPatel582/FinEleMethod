param(
    [Parameter(Mandatory = $true)]
    [string]$InstallerFile,

    [Parameter(Mandatory = $true)]
    [string]$ChecksumFile,

    [string]$ExpectedVersion = "0.1.0.0",
    [string]$ExpectedPublisher = "Anoop Patel"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$installer = (Resolve-Path -LiteralPath $InstallerFile).Path
$checksum = (Resolve-Path -LiteralPath $ChecksumFile).Path
$installerItem = Get-Item -LiteralPath $installer
$checksumText = (Get-Content -LiteralPath $checksum -Raw).Trim()

if ($checksumText -notmatch '^([0-9a-fA-F]{64})\s{2}(.+)$') {
    throw "Checksum file must contain a SHA-256 value, two spaces, and the installer filename."
}
if ($Matches[2] -ne $installerItem.Name) {
    throw "Checksum filename '$($Matches[2])' does not match '$($installerItem.Name)'."
}

$expectedHash = $Matches[1].ToLowerInvariant()
$actualHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $installer).Hash.ToLowerInvariant()
if ($actualHash -ne $expectedHash) {
    throw "Installer SHA-256 does not match its checksum file."
}

$versionInfo = $installerItem.VersionInfo
if ($versionInfo.FileVersion.Trim() -ne $ExpectedVersion -or
    $versionInfo.ProductVersion.Trim() -ne $ExpectedVersion) {
    throw "Installer version metadata does not match $ExpectedVersion."
}
if ($versionInfo.CompanyName.Trim() -ne $ExpectedPublisher) {
    throw "Installer publisher metadata does not match '$ExpectedPublisher'."
}
if ($versionInfo.ProductName.Trim() -ne "FinEleMethod" -or
    $versionInfo.FileDescription.Trim() -ne "FinEleMethod Windows Installer") {
    throw "Installer product metadata is incomplete or incorrect."
}

Write-Host "Windows installer checksum and metadata verification passed."
Write-Host "Installer: $installer"
Write-Host "Version: $ExpectedVersion"
Write-Host "Publisher: $ExpectedPublisher"
Write-Host "SHA256: $actualHash"
