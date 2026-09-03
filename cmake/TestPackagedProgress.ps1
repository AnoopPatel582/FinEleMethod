$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
. (Join-Path $PSScriptRoot 'PackagedProcess.ps1')
$valid = '{"protocolVersion":1,"event":"analysis-progress","state":"cancelled","message":"Cancelled."}'
if ((@(Read-ProgressStates "$valid`r`n") -join ',') -ne 'cancelled') { throw 'Valid progress rejected.' }
foreach ($text in @('not json', '{}',
    $valid.Replace('"protocolVersion":1','"protocolVersion":2'),
    $valid.Replace('analysis-progress','unknown-event'),
    $valid.Replace('"cancelled"','"unknown-state"'),
    $valid.Replace('Cancelled.',''))) {
    $rejected = $false
    try { Read-ProgressStates $text | Out-Null } catch { $rejected = $true }
    if (-not $rejected) { throw "Invalid progress accepted: $text" }
}
Write-Host 'Packaged progress validation regression checks passed.'
