param(
    [Parameter(Mandatory = $true)]
    [string]$StageDirectory,

    [Parameter(Mandatory = $true)]
    [string]$ReleaseDirectory
)

$ErrorActionPreference = "Stop"

$stage = (Resolve-Path -LiteralPath $StageDirectory).Path
$release = (Resolve-Path -LiteralPath $ReleaseDirectory).Path
$solver = Join-Path $stage "FinEleMethod.exe"
$workbench = Join-Path $stage "FinEleMethodGui.exe"
$example = Join-Path $stage "examples\abaqus\q4_tension.inp"

$requiredDocuments = @(
    "START_HERE.md", "README.md", "docs\WINDOWS_QUICK_START.md",
    "docs\BEGINNERS_GUIDE.md", "docs\ARCHITECTURE.md", "docs\PROJECT_DECISIONS.md",
    "docs\GUI_ACCEPTANCE.md", "docs\formulations\Q4.md", "docs\formulations\H8.md",
    "docs\formulations\SYSTEM_SOLUTION.md", "docs\benchmarks\cantilever_beam.md",
    "docs\benchmarks\plate_with_hole.md", "docs\benchmarks\h8_compression.md"
)
foreach ($relativePath in $requiredDocuments) {
    $document = Join-Path $stage $relativePath
    if (-not (Test-Path -LiteralPath $document -PathType Leaf) -or
        (Get-Item -LiteralPath $document).Length -eq 0) {
        throw "Staged application is missing required documentation: $relativePath"
    }
}

foreach ($requiredPath in @($solver, $workbench, $example, (Join-Path $stage "z.dll"))) {
    if (-not (Test-Path -LiteralPath $requiredPath -PathType Leaf)) {
        throw "Staged application is missing required file: $requiredPath"
    }
}

$releaseDlls = @(
    Get-ChildItem -LiteralPath $release -Filter "*.dll" -File |
        Select-Object -ExpandProperty Name |
        Sort-Object
)
$stagedDlls = @(
    Get-ChildItem -LiteralPath $stage -Filter "*.dll" -File |
        Select-Object -ExpandProperty Name |
        Sort-Object
)
$dllDifference = @(Compare-Object $releaseDlls $stagedDlls)
if ($dllDifference.Count -ne 0) {
    $differenceText = $dllDifference | Out-String
    throw "Staged DLL set does not match the Release app-local DLL set:`n$differenceText"
}

& $solver --inspect $example
if ($LASTEXITCODE -ne 0) {
    throw "Staged command-line model inspection failed with exit code $LASTEXITCODE."
}

& (Join-Path $PSScriptRoot "VerifyStagedSolver.ps1") -StageDirectory $stage
& (Join-Path $PSScriptRoot "VerifyStagedRequest.ps1") -StageDirectory $stage

$startInfo = [System.Diagnostics.ProcessStartInfo]::new()
$startInfo.FileName = $workbench
$startInfo.WorkingDirectory = $stage
$startInfo.UseShellExecute = $false
$startInfo.WindowStyle = [System.Diagnostics.ProcessWindowStyle]::Minimized
$startInfo.Environment.Clear()
$startInfo.Environment["SystemRoot"] = $env:SystemRoot
$startInfo.Environment["WINDIR"] = $env:WINDIR
$startInfo.Environment["PATH"] = "$env:SystemRoot\System32"

$process = [System.Diagnostics.Process]::Start($startInfo)
try {
    Start-Sleep -Seconds 3
    if ($process.HasExited) {
        throw "Staged workbench exited early with code $($process.ExitCode)."
    }
    $process.Refresh()
    if ($process.MainWindowTitle -ne "FinEleMethod") {
        throw "Staged workbench did not create the expected FinEleMethod window. " +
            "Observed title: '$($process.MainWindowTitle)'."
    }
}
finally {
    if (-not $process.HasExited) {
        $process.Kill()
        $process.WaitForExit()
    }
    $process.Dispose()
}

Write-Host "Staged application verification passed with $($stagedDlls.Count) app-local DLLs."
