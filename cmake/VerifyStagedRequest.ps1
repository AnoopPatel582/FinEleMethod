param([Parameter(Mandatory = $true)][string]$StageDirectory)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
. (Join-Path $PSScriptRoot 'PackagedProcess.ps1')
. (Join-Path $PSScriptRoot 'VtuVerification.ps1')
$stage = (Resolve-Path -LiteralPath $StageDirectory).Path
$solver = Join-Path $stage 'FinEleMethod.exe'
$fixture = Join-Path ([System.IO.Path]::GetTempPath()) ('FinEleMethod request checks ' + [guid]::NewGuid())
New-Item -ItemType Directory -Path $fixture | Out-Null
foreach ($model in @('q4_tension','q4_plane_strain_tension','h8_compression')) {
    $run = Join-Path $fixture $model
    New-Item -ItemType Directory -Path (Join-Path $run 'input'), (Join-Path $run 'results') | Out-Null
    Copy-Item -LiteralPath (Join-Path $stage "examples/abaqus/$model.inp") -Destination (Join-Path $run 'input/model.inp')
    $request = Join-Path $run 'analysis-request.json'
    [System.IO.File]::WriteAllText($request, (@{
        protocolVersion=1; inputFile='input/model.inp'; resultFile='results/model.vtu'; summaryFile='results/analysis-summary.json'
    } | ConvertTo-Json))
    $result = Invoke-PackagedSolver $solver $fixture @('--request', $request)
    [System.IO.File]::WriteAllText((Join-Path $run 'stdout.jsonl'), $result.Output)
    [System.IO.File]::WriteAllText((Join-Path $run 'stderr.txt'), $result.Error)
    if ($result.ExitCode -ne 0 -or $result.Error -ne '') { throw "Packaged request failed: $model ($($result.ExitCode)) $($result.Error)" }
    $states = @(Read-ProgressStates $result.Output)
    if (($states -join ',') -ne 'preparing,executing,writing-results,completed') {
        throw "Incorrect packaged request lifecycle: $($states -join ',')"
    }
    $state = Get-Content -LiteralPath (Join-Path $run 'analysis-state.json') -Raw | ConvertFrom-Json
    $summary = Get-Content -LiteralPath (Join-Path $run 'results/analysis-summary.json') -Raw | ConvertFrom-Json
    if ($state.schemaVersion -ne 1 -or $state.state -ne 'completed' -or
        $summary.protocolVersion -ne 1 -or $summary.status -ne 'completed' -or
        [System.IO.Path]::GetFullPath($summary.inputFile) -ne (Join-Path $run 'input/model.inp') -or
        [System.IO.Path]::GetFullPath($summary.resultFile) -ne (Join-Path $run 'results/model.vtu')) {
        throw "Incorrect packaged request completion files: $model"
    }
    [xml]$vtu = Get-Content -LiteralPath (Join-Path $run 'results/model.vtu') -Raw
    Assert-ExampleSolution $vtu.VTKFile.UnstructuredGrid.Piece $model
    Write-Host "Verified packaged request lifecycle and analytical results: $model"
}
foreach ($case in @(
    @{Name='cancelled'; Version=1; Exit=6; State='cancelled'; Persisted=$true},
    @{Name='missing-input'; Version=1; Exit=2; State='failed'; Persisted=$true},
    @{Name='unsupported-version'; Version=2; Exit=2; State='failed'; Persisted=$false}
)) {
    $run = Join-Path $fixture $case.Name
    New-Item -ItemType Directory -Path (Join-Path $run 'input'), (Join-Path $run 'results') | Out-Null
    $request = Join-Path $run 'analysis-request.json'
    [System.IO.File]::WriteAllText($request, (@{
        protocolVersion=$case.Version; inputFile='input/model.inp'; resultFile='results/model.vtu'; summaryFile='results/analysis-summary.json'
    } | ConvertTo-Json))
    if ($case.Name -eq 'cancelled') {
        Copy-Item -LiteralPath (Join-Path $stage 'examples/abaqus/q4_tension.inp') -Destination (Join-Path $run 'input/model.inp')
        [System.IO.File]::WriteAllText((Join-Path $run 'cancellation-requested.flag'), 'Cancellation requested.')
    }
    $result = Invoke-PackagedSolver $solver $fixture @('--request', $request)
    [System.IO.File]::WriteAllText((Join-Path $run 'stdout.jsonl'), $result.Output)
    [System.IO.File]::WriteAllText((Join-Path $run 'stderr.txt'), $result.Error)
    $states = @(Read-ProgressStates $result.Output)
    if ($result.ExitCode -ne $case.Exit -or ($states -join ',') -ne "preparing,$($case.State)") {
        throw "Incorrect packaged error/cancellation lifecycle: $($case.Name)"
    }
    if (($case.State -eq 'cancelled' -and $result.Error -ne '') -or
        ($case.State -eq 'failed' -and [string]::IsNullOrWhiteSpace($result.Error))) {
        throw "Incorrect error-channel behavior: $($case.Name)"
    }
    foreach ($file in @('results/model.vtu','results/analysis-summary.json')) {
        if (Test-Path -LiteralPath (Join-Path $run $file)) { throw "Unsuccessful run created completion output: $file" }
    }
    if ($case.Persisted) {
        $state = Get-Content -LiteralPath (Join-Path $run 'analysis-state.json') -Raw | ConvertFrom-Json
        if ($state.schemaVersion -ne 1 -or $state.state -ne $case.State) {
            throw "Incorrect persisted unsuccessful state: $($case.Name)"
        }
    }
    Write-Host "Verified packaged unsuccessful request: $($case.Name)"
}
Write-Host "Packaged request evidence retained: $fixture"
