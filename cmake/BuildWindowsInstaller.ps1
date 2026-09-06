param(
    [string]$Version = "0.1.0",
    [string]$StageDirectory = ".\out\install\windows-msvc-release",
    [string]$ReleaseDirectory = ".\out\build\windows-msvc\Release",
    [string]$OutputDirectory = ".\out\installer",
    [string]$InnoCompiler,
    [string]$VCRedistPath
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$repository = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path

function Resolve-ExistingFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "$Description was not found: $Path"
    }
    return (Resolve-Path -LiteralPath $Path).Path
}

if ($Version -notmatch '^\d+\.\d+\.\d+$') {
    throw "Version must contain exactly three numeric components, for example 0.1.0."
}
$versionInfoVersion = "$Version.0"

$stage = (Resolve-Path -LiteralPath $StageDirectory).Path
$release = (Resolve-Path -LiteralPath $ReleaseDirectory).Path
$output = [System.IO.Path]::GetFullPath($OutputDirectory)
$installerScript = Resolve-ExistingFile `
    -Path (Join-Path $repository "installer\FinEleMethod.iss") `
    -Description "Inno Setup definition"

if ([string]::IsNullOrWhiteSpace($InnoCompiler)) {
    $compilerCandidates = @(
        (Join-Path $env:LOCALAPPDATA "Programs\Inno Setup 6\ISCC.exe"),
        (Join-Path $env:ProgramFiles "Inno Setup 6\ISCC.exe")
    )
    $InnoCompiler = $compilerCandidates |
        Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } |
        Select-Object -First 1
}
if ([string]::IsNullOrWhiteSpace($InnoCompiler)) {
    throw "Inno Setup 6 compiler was not found. Install JRSoftware.InnoSetup or pass -InnoCompiler."
}
$compiler = Resolve-ExistingFile -Path $InnoCompiler -Description "Inno Setup compiler"

if ([string]::IsNullOrWhiteSpace($VCRedistPath)) {
    $visualStudioRoot = Join-Path $env:ProgramFiles "Microsoft Visual Studio"
    $VCRedistPath = Get-ChildItem -LiteralPath $visualStudioRoot `
        -Filter "vc_redist.x64.exe" -File -Recurse -ErrorAction SilentlyContinue |
        Sort-Object { [version]$_.VersionInfo.FileVersion } -Descending |
        Select-Object -First 1 -ExpandProperty FullName
}
if ([string]::IsNullOrWhiteSpace($VCRedistPath)) {
    throw "Microsoft Visual C++ x64 Redistributable was not found. Pass -VCRedistPath."
}
$redist = Resolve-ExistingFile -Path $VCRedistPath -Description "Visual C++ x64 Redistributable"

$signature = Get-AuthenticodeSignature -LiteralPath $redist
if ($signature.Status -ne [System.Management.Automation.SignatureStatus]::Valid -or
    $null -eq $signature.SignerCertificate -or
    $signature.SignerCertificate.Subject -notmatch 'O=Microsoft Corporation') {
    throw "Visual C++ Redistributable does not have a valid Microsoft signature: $redist"
}

$redistVersion = [version](Get-Item -LiteralPath $redist).VersionInfo.FileVersion
if ($redistVersion.Major -ne 14) {
    throw "Expected a Microsoft Visual C++ v14 x64 Redistributable, found $redistVersion."
}

& (Join-Path $PSScriptRoot "VerifyStagedApplication.ps1") `
    -StageDirectory $stage `
    -ReleaseDirectory $release

$buildInfo = & (Join-Path $stage "FinEleMethod.exe") --build-info
if ($LASTEXITCODE -ne 0 -or $buildInfo[0] -ne "FinEleMethod $Version") {
    throw "Staged solver version does not match installer version $Version."
}

New-Item -ItemType Directory -Force -Path $output | Out-Null
$arguments = @(
    "/DAppVersion=$Version",
    "/DVersionInfoVersion=$versionInfoVersion",
    "/DStageDirectory=$stage",
    "/DOutputDirectory=$output",
    "/DVCRedistPath=$redist",
    "/DVCRuntimeMajor=$($redistVersion.Major)",
    "/DVCRuntimeMinor=$($redistVersion.Minor)",
    "/DVCRuntimeBuild=$($redistVersion.Build)",
    "/DVCRuntimeRevision=$($redistVersion.Revision)",
    $installerScript
)

& $compiler @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Inno Setup compiler failed with exit code $LASTEXITCODE."
}

$installer = Join-Path $output "FinEleMethod-$Version-windows-x64-setup.exe"
if (-not (Test-Path -LiteralPath $installer -PathType Leaf)) {
    throw "Expected installer was not created: $installer"
}

$installerItem = Get-Item -LiteralPath $installer
$hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $installer).Hash.ToLowerInvariant()
$checksumFile = "$installer.sha256"
Set-Content -LiteralPath $checksumFile -Encoding ascii -NoNewline `
    -Value "$hash  $($installerItem.Name)`n"

Write-Host "Windows installer created and checksum generated."
Write-Host "Installer: $($installerItem.FullName)"
Write-Host "Size: $($installerItem.Length) bytes"
Write-Host "SHA256: $hash"
Write-Host "Bundled VC Runtime: $redistVersion"
