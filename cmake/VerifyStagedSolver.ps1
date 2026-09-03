param(
    [Parameter(Mandatory = $true)]
    [string]$StageDirectory
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
. (Join-Path $PSScriptRoot 'VtuVerification.ps1')

$stage = (Resolve-Path -LiteralPath $StageDirectory).Path
$solver = Join-Path $stage "FinEleMethod.exe"
$cases = @(
    @{ Name = "q4_tension"; Type = "q4-plane-stress"; Nodes = 4 },
    @{ Name = "q4_plane_strain_tension"; Type = "q4-plane-strain"; Nodes = 4 },
    @{ Name = "h8_compression"; Type = "h8-three-dimensional"; Nodes = 8 }
)
foreach ($required in @($solver) + @($cases | ForEach-Object {
    Join-Path $stage "examples\abaqus\$($_.Name).inp"
})) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Staged solver verification requires: $required"
    }
}

$buildInfo = & $solver --build-info
if ($LASTEXITCODE -ne 0 -or
    ($buildInfo -join "`n") -notmatch '(?m)^Configuration: Release\r?$' -or
    ($buildInfo -join "`n") -notmatch '(?m)^Architecture: 64-bit\r?$') {
    throw "Staged solver must report a Release, 64-bit build."
}
$buildInfo | ForEach-Object { Write-Host $_ }

# Fresh output paths prevent stale results from making a broken executable pass.
# Retain the small outputs for diagnosis; never write into the shipped package.
$output = Join-Path ([System.IO.Path]::GetTempPath()) ("FinEleMethod solver check " + [guid]::NewGuid())
New-Item -ItemType Directory -Path $output | Out-Null
Write-Host "Staged solver verification outputs: $output"

foreach ($case in $cases) {
    $inputFile = Join-Path $stage "examples\abaqus\$($case.Name).inp"
    $resultFile = Join-Path $output "$($case.Name).vtu"
    $summaryFile = Join-Path $output "$($case.Name).json"
    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $solver
    $startInfo.WorkingDirectory = $output
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    foreach ($argument in @("--input", $inputFile, "--output", $resultFile, "--summary", $summaryFile)) {
        $startInfo.ArgumentList.Add($argument)
    }
    $startInfo.Environment.Clear()
    $startInfo.Environment["SystemRoot"] = $env:SystemRoot
    $startInfo.Environment["WINDIR"] = $env:WINDIR
    $startInfo.Environment["PATH"] = "$env:SystemRoot\System32"
    $process = [System.Diagnostics.Process]::Start($startInfo)
    try {
        if (-not $process.WaitForExit(60000)) {
            throw "Packaged solver timed out for $($case.Name)."
        }
        if ($process.ExitCode -ne 0) {
            throw "Packaged solver failed for $($case.Name), exit code $($process.ExitCode)."
        }
    }
    finally {
        if (-not $process.HasExited) {
            $process.Kill()
            $process.WaitForExit()
        }
        $process.Dispose()
    }

    $summary = Get-Content -LiteralPath $summaryFile -Raw | ConvertFrom-Json
    if ($summary.protocolVersion -ne 1 -or $summary.status -ne "completed" -or
        $summary.analysisType -ne $case.Type -or $summary.nodeCount -ne $case.Nodes -or
        $summary.elementCount -ne 1 -or
        [System.IO.Path]::GetFullPath($summary.inputFile) -ne $inputFile -or
        [System.IO.Path]::GetFullPath($summary.resultFile) -ne $resultFile) {
        throw "Unexpected packaged analysis summary for $($case.Name)."
    }
    [xml]$vtu = Get-Content -LiteralPath $resultFile -Raw
    $piece = $vtu.VTKFile.UnstructuredGrid.Piece
    if ($vtu.VTKFile.type -ne "UnstructuredGrid" -or
        [int]$piece.NumberOfPoints -ne $case.Nodes -or [int]$piece.NumberOfCells -ne 1) {
        throw "Unexpected packaged VTU mesh for $($case.Name)."
    }
    foreach ($field in @("Displacement", "ReactionForce")) {
        Read-VtuField $piece PointData $field $case.Nodes 3 | Out-Null
    }
    $components = switch ($case.Type) {
        'q4-plane-stress' { 3 }
        'q4-plane-strain' { 4 }
        'h8-three-dimensional' { 6 }
    }
    foreach ($field in @('Stress', 'Strain')) {
        Read-VtuField $piece CellData $field 1 $components | Out-Null
    }
    Read-VtuField $piece CellData PrincipalStress 1 3 | Out-Null
    Read-VtuField $piece CellData VonMises 1 1 | Out-Null
    Write-Host "Verified packaged solve and output structure: $($case.Name)"
}
