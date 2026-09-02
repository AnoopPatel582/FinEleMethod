param(
    [string]$BuildDirectory = ".\out\build\windows-msvc",
    [string]$ClangTidy,
    [ValidateRange(1, 16)]
    [int]$Jobs = 4
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ClangTidy)) {
    $pathCommand = Get-Command "clang-tidy" -ErrorAction SilentlyContinue
    if ($pathCommand) {
        $ClangTidy = $pathCommand.Source
    }
    else {
        $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
        $installationPaths = @(& $vswhere -all -products "*" -property installationPath)
        foreach ($installationPath in $installationPaths) {
            $candidate = Join-Path $installationPath "VC\Tools\Llvm\x64\bin\clang-tidy.exe"
            if (Test-Path -LiteralPath $candidate -PathType Leaf) {
                $ClangTidy = $candidate
                break
            }
        }
    }
}
if ([string]::IsNullOrWhiteSpace($ClangTidy) -or
    -not (Test-Path -LiteralPath $ClangTidy -PathType Leaf)) {
    throw "Could not locate clang-tidy."
}

$vcpkgRoot = Join-Path (Resolve-Path -LiteralPath $BuildDirectory).Path "vcpkg_installed\x64-windows"
$vcpkgInclude = Join-Path $vcpkgRoot "include"
$wxGeneratedInclude = Join-Path $vcpkgRoot "lib\mswu"
foreach ($requiredDirectory in @($vcpkgInclude, $wxGeneratedInclude)) {
    if (-not (Test-Path -LiteralPath $requiredDirectory -PathType Container)) {
        throw "Required analysis include directory does not exist: $requiredDirectory"
    }
}

$files = @(git ls-files -- "src/*.cpp" "apps/*.cpp")
if ($LASTEXITCODE -ne 0 -or $files.Count -eq 0) {
    throw "Could not list production C++ source files."
}

$results = @($files | ForEach-Object -Parallel {
    $arguments = @(
        $_,
        "--quiet",
        "--header-filter=.*[\\/](include|src|apps)[\\/].*",
        "--",
        "-std=c++20",
        "-DUNICODE",
        "-D_UNICODE",
        "-DWIN32",
        "-D_WINDOWS",
        "-Iinclude",
        "-isystem",
        $using:vcpkgInclude,
        "-isystem",
        $using:wxGeneratedInclude
    )
    $diagnostics = @(& $using:ClangTidy @arguments 2>&1)
    $exitCode = $LASTEXITCODE
    foreach ($diagnostic in $diagnostics) {
        Write-Host $diagnostic
    }
    [pscustomobject]@{ File = $_; ExitCode = $exitCode }
} -ThrottleLimit $Jobs)

$failures = @($results | Where-Object ExitCode -ne 0)
if ($failures.Count -ne 0) {
    $failedFiles = ($failures.File -join ", ")
    throw "clang-tidy failed for $($failures.Count) file(s): $failedFiles"
}

Write-Host "clang-tidy verification passed for $($files.Count) production source files."
