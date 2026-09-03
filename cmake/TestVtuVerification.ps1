$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
. (Join-Path $PSScriptRoot 'VtuVerification.ps1')

function Test-Field([string]$Text, [string]$Components = '3', [string]$Format = 'ascii') {
    [xml]$document = "<Piece><PointData><DataArray Name='Displacement' type='Float64' NumberOfComponents='$Components' format='$Format'>$Text</DataArray></PointData></Piece>"
    return ,(Read-VtuField $document.Piece PointData Displacement 1 3)
}
$values = Test-Field '1.25e-2 -2 0'
if ($values.Count -ne 3 -or $values[0] -ne 0.0125 -or $values[1] -ne -2) {
    throw 'Valid field parsing failed.'
}
foreach ($text in @('', '1 2', '1 2 3 4', 'NaN 0 0', 'Infinity 0 0', '1e999 0 0', 'bad 0 0')) {
    $rejected = $false
    try { Test-Field $text | Out-Null } catch { $rejected = $true }
    if (-not $rejected) { throw "Invalid field accepted: $text" }
}
foreach ($case in @(@{Components='2'; Format='ascii'}, @{Components='3'; Format='binary'})) {
    $rejected = $false
    try { Test-Field '1 2 3' @case | Out-Null } catch { $rejected = $true }
    if (-not $rejected) { throw 'Invalid components or encoding accepted.' }
}
foreach ($body in @('', "<DataArray Name='Displacement'/><DataArray Name='Displacement'/>")) {
    [xml]$document = "<Piece><PointData>$body</PointData></Piece>"
    $rejected = $false
    try { Read-VtuField $document.Piece PointData Displacement 1 3 | Out-Null } catch { $rejected = $true }
    if (-not $rejected) { throw 'Missing or duplicate field accepted.' }
}
Write-Host 'VTU field validation regression checks passed.'
