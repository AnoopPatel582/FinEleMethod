param(
    [string]$ClangFormat
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ClangFormat)) {
    $pathCommand = Get-Command "clang-format" -ErrorAction SilentlyContinue
    if ($pathCommand) {
        $ClangFormat = $pathCommand.Source
    }
    else {
        $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
        if (-not (Test-Path -LiteralPath $vswhere -PathType Leaf)) {
            throw "Could not find clang-format on PATH or locate Visual Studio with vswhere."
        }

        $installationPaths = @(& $vswhere -all -products "*" -property installationPath)
        if ($LASTEXITCODE -ne 0) {
            throw "Could not query Visual Studio installation paths."
        }

        foreach ($installationPath in $installationPaths) {
            $candidate = Join-Path $installationPath "VC\Tools\Llvm\x64\bin\clang-format.exe"
            if (Test-Path -LiteralPath $candidate -PathType Leaf) {
                $ClangFormat = $candidate
                break
            }
        }

        if ([string]::IsNullOrWhiteSpace($ClangFormat)) {
            throw "Could not locate a Visual Studio installation containing clang-format."
        }
    }
}

if (-not (Test-Path -LiteralPath $ClangFormat -PathType Leaf)) {
    throw "clang-format executable was not found: $ClangFormat"
}

$files = @(git ls-files -- "*.cpp" "*.hpp")
if ($LASTEXITCODE -ne 0) {
    throw "Failed to list tracked C++ files."
}
if ($files.Count -eq 0) {
    throw "No tracked C++ files were found."
}

& $ClangFormat --dry-run --Werror --style=file $files
if ($LASTEXITCODE -ne 0) {
    throw "clang-format reported one or more incorrectly formatted C++ files."
}

Write-Host "Formatting verification passed for $($files.Count) C++ files."
