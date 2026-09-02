[CmdletBinding()]
param(
    [string]$OutputDirectory = "out/docs/api",
    [string]$ToolDirectory = "out/tools/doxygen-1.18.0"
)

$ErrorActionPreference = "Stop"
$repositoryRoot = Split-Path -Parent $PSScriptRoot
$doxygen = Get-Command doxygen -ErrorAction SilentlyContinue

if (-not $doxygen) {
    $toolRoot = Join-Path $repositoryRoot $ToolDirectory
    $executable = Join-Path $toolRoot "doxygen.exe"

    if (-not (Test-Path -LiteralPath $executable)) {
        $archive = Join-Path $env:TEMP "doxygen-1.18.0.windows.x64.bin.zip"
        $downloadUrl = "https://www.doxygen.nl/files/doxygen-1.18.0.windows.x64.bin.zip"
        $expectedHash = "e84f54cfd49ef06b0b16536056dbec0c496323de28abcce53a4269463de35eaf"

        Invoke-WebRequest -Uri $downloadUrl -OutFile $archive
        $actualHash = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actualHash -ne $expectedHash) {
            throw "Doxygen archive checksum verification failed."
        }

        New-Item -ItemType Directory -Path $toolRoot -Force | Out-Null
        Expand-Archive -LiteralPath $archive -DestinationPath $toolRoot -Force
        Remove-Item -LiteralPath $archive
    }

    $doxygenPath = $executable
}
else {
    $doxygenPath = $doxygen.Source
}

$resolvedOutput = Join-Path $repositoryRoot $OutputDirectory
New-Item -ItemType Directory -Path $resolvedOutput -Force | Out-Null
$env:FINELEMETHOD_DOXYGEN_OUTPUT = $resolvedOutput.Replace("\", "/")

Push-Location $repositoryRoot
try {
    & $doxygenPath "Doxyfile"
    if ($LASTEXITCODE -ne 0) {
        throw "Doxygen failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
    Remove-Item Env:FINELEMETHOD_DOXYGEN_OUTPUT -ErrorAction SilentlyContinue
}

$index = Join-Path $resolvedOutput "html/index.html"
if (-not (Test-Path -LiteralPath $index)) {
    throw "Doxygen did not create the expected API index: $index"
}

Write-Host "API documentation created at $index"
