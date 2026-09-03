function Invoke-PackagedSolver([string]$Solver, [string]$WorkingDirectory, [string[]]$Arguments) {
    $info = [System.Diagnostics.ProcessStartInfo]::new()
    $info.FileName = $Solver
    $info.WorkingDirectory = $WorkingDirectory
    $info.UseShellExecute = $false
    $info.CreateNoWindow = $true
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    foreach ($argument in $Arguments) { $info.ArgumentList.Add($argument) }
    $info.Environment.Clear()
    $info.Environment['SystemRoot'] = $env:SystemRoot
    $info.Environment['WINDIR'] = $env:WINDIR
    $info.Environment['PATH'] = "$env:SystemRoot\System32"
    $process = [System.Diagnostics.Process]::Start($info)
    try {
        # Drain both streams concurrently so neither full pipe can deadlock exit.
        $stdout = $process.StandardOutput.ReadToEndAsync()
        $stderr = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit(60000)) { throw 'Packaged request solver timed out.' }
        return [pscustomobject]@{
            ExitCode = $process.ExitCode
            Output = $stdout.GetAwaiter().GetResult()
            Error = $stderr.GetAwaiter().GetResult()
        }
    }
    finally {
        if (-not $process.HasExited) { $process.Kill(); $process.WaitForExit() }
        $process.Dispose()
    }
}

function Read-ProgressStates([string]$Text) {
    foreach ($line in ($Text -split '\r?\n' | Where-Object { $_.Trim() -ne '' })) {
        $record = $line | ConvertFrom-Json
        if ($record.protocolVersion -ne 1 -or $record.event -ne 'analysis-progress' -or
            [string]::IsNullOrWhiteSpace($record.message) -or
            $record.state -notin @('preparing','executing','writing-results','completed','failed','cancelled')) {
            throw 'Invalid packaged progress record.'
        }
        $record.state
    }
}
