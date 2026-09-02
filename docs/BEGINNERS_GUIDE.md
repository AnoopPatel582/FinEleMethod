# Beginner's Guide

This guide takes a new Windows user from the source repository to a solved
finite element model. No commercial finite element software is required.

## What the current application does

FinEleMethod is currently a command-line solver. It reads supported ABAQUS
`.inp` files, solves the model, and writes a `.vtu` result that can be viewed in
ParaView.

Supported element types are:

| ABAQUS type | Analysis |
| --- | --- |
| `CPS4` | Four-node Q4 plane stress |
| `CPE4` | Four-node Q4 plane strain |
| `C3D8` | Eight-node H8 three-dimensional solid |

## 1. Install the required tools

Install these tools before building:

1. **Git for Windows** for downloading and updating the source repository.
2. **Visual Studio Community 2026** with the **Desktop development with C++**
   workload.
3. In the Visual Studio Installer, keep the CMake and vcpkg components selected.
4. **ParaView** for viewing `.vtu` result files.

Visual Studio supplies the MSVC compiler and CMake used by this project. The
vcpkg manifest downloads the approved C++ libraries automatically during the
first configuration.

## 2. Clone the repository

Open **Developer PowerShell for Visual Studio 2026** and choose a parent folder.
For example:

```powershell
cd D:\
git clone https://github.com/AnoopPatel582/FinEleMethod.git
cd .\FinEleMethod
```

All remaining commands in this guide assume the terminal is at the repository
root—the folder containing `CMakeLists.txt`, `CMakePresets.json`, and
`vcpkg.json`.

## 3. Configure vcpkg

Set `VCPKG_ROOT` for the current PowerShell session. The default Visual Studio
Community location is:

```powershell
$env:VCPKG_ROOT = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\vcpkg"
```

Confirm that the location is correct:

```powershell
Test-Path "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
```

The command must print `True`. If it prints `False`, open Visual Studio Installer
and confirm that vcpkg is installed, or change `VCPKG_ROOT` to the actual folder
containing `vcpkg.exe` and the `scripts` directory.

## 4. Configure the project

Run:

```powershell
cmake --preset windows-msvc
```

During the first configuration, vcpkg may download and build GoogleTest,
nlohmann/json, spdlog, and their supporting packages. Later configurations reuse
the local package cache.

A successful configuration ends with a message similar to:

```text
Build files have been written to: ...\FinEleMethod\out\build\windows-msvc
```

## 5. Build the application

Run:

```powershell
cmake --build --preset windows-msvc-debug
```

The command compiles the solver and tests. The Debug executable is created at:

```text
out\build\windows-msvc\Debug\FinEleMethod.exe
```

Confirm that the program starts:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe --help
```

For an optimized build intended for demonstrations or later packaging, run:

```powershell
cmake --build --preset windows-msvc-release
```

The Release command-line solver and workbench are created in
`out\build\windows-msvc\Release\`.

To collect the application and its runtime files into a clean folder, run:

```powershell
cmake --install .\out\build\windows-msvc `
  --config Release `
  --prefix .\out\install\windows-msvc-release
```

The staged application is then available under
`out\install\windows-msvc-release\`. This development-stage folder assumes the
Microsoft Visual C++ Runtime is installed; the future installer will manage that
prerequisite.

To create a ZIP from the staged application, run:

```powershell
.\cmake\CreateWindowsArchive.ps1 `
  -StageDirectory .\out\install\windows-msvc-release `
  -OutputFile .\out\package\FinEleMethod-windows-x64.zip
```

## 6. Run the automated tests

Run:

```powershell
ctest --preset windows-msvc-debug
```

The final result must report that 100% of tests passed. If a test fails, keep the
complete failure output; it identifies the affected feature and should be fixed
before continuing development.

## 7. Solve your first model

Create an output folder if it does not already exist:

```powershell
New-Item -ItemType Directory -Force .\out\results
```

Solve the supplied Q4 plane-stress tension model:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\q4_tension.inp `
  --output .\out\results\q4_tension.vtu `
  --summary .\out\results\q4_tension-summary.json
```

A successful run prints the input path, VTU result path, and JSON summary path.
It creates:

- `out\results\q4_tension.vtu`, containing visualization results.
- `out\results\q4_tension-summary.json`, containing a versioned completion
  record for application integration.

Display the summary in PowerShell:

```powershell
Get-Content .\out\results\q4_tension-summary.json
```

The analytical solution for this example has a right-edge X displacement of
`0.01`, total X reaction of `-10.0`, and uniform X stress of `10.0`.

## 8. View the result in ParaView

1. Start ParaView.
2. Select **File → Open**.
3. Choose `out\results\q4_tension.vtu`.
4. Click **Apply** in the Properties panel.
5. Use the coloring list above the graphics view to select `VonMises`, `Stress`,
   `Strain`, or `Displacement`.
6. To display deformation, apply **Warp By Vector**, select `Displacement` as
   the vector, and click **Apply**.
7. Increase the warp scale factor when the physical displacement is too small to
   see clearly.

The VTU file contains the undeformed mesh coordinates and the computed result
fields. Warp By Vector creates a visual deformation; it does not modify the
solver result.

## 9. Run the other verification models

### Q4 plane strain

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\q4_plane_strain_tension.inp `
  --output .\out\results\q4_plane_strain_tension.vtu
```

### Q4 cantilever beam

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\q4_cantilever.inp `
  --output .\out\results\q4_cantilever.vtu
```

### Q4 plate with a hole

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\q4_plate_with_hole.inp `
  --output .\out\results\q4_plate_with_hole.vtu
```

### H8 block compression

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\h8_compression.inp `
  --output .\out\results\h8_compression.vtu
```

## 10. Open the project in Visual Studio

Visual Studio can open this repository directly as a CMake project:

1. Start Visual Studio.
2. Select **Open a local folder**.
3. Choose the `FinEleMethod` repository folder.
4. Wait for CMake configuration to finish.
5. Select `FinEleMethod.exe` as the startup item when you want to debug the CLI.

Set `VCPKG_ROOT` in the environment before starting Visual Studio if automatic
configuration cannot locate the toolchain file.

## Common problems

### CMake cannot find the compiler

Open **Developer PowerShell for Visual Studio 2026**, not ordinary PowerShell,
and confirm that the **Desktop development with C++** workload is installed.

### CMake cannot find the vcpkg toolchain

Repeat the `VCPKG_ROOT` command and use `Test-Path` from section 3. Environment
variables set in one terminal are not automatically available in a different
terminal.

### Visual Studio Code shows red underlines while Visual Studio builds correctly

Configure the CMake project in VS Code and make sure its C++ extension uses the
CMake-generated configuration. A red underline can come from an unconfigured
editor even when the MSVC build is valid.

### The solver reports an input-parsing error

Check that the input uses a currently supported element type and that its nodes,
materials, solid sections, constraints, and loads follow the supported ABAQUS
syntax. Start from one of the files in `examples\abaqus` and change it gradually.

### ParaView displays an empty view

Select the dataset in the Pipeline Browser and click **Apply**. Then use **Reset
Camera** if the model is outside the current view.

## Safe development workflow

For every new feature:

1. Create or update focused tests.
2. Build with MSVC.
3. Run the relevant tests and then the full suite.
4. Review the changed files.
5. Commit one cohesive increment.
6. Push it to GitHub and confirm continuous integration passes.

Important architecture decisions belong in
[PROJECT_DECISIONS.md](PROJECT_DECISIONS.md). The implemented component flow is
described in [ARCHITECTURE.md](ARCHITECTURE.md).
