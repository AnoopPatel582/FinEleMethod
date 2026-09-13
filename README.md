# FinEleMethod

FinEleMethod is a custom finite element solver and native Windows engineering
workbench developed from scratch in C++20.

## Project status

- Current version: **0.1.0**.
- Target platform: **Windows 10 or later, x64**.
- Toolchain: **MSVC, CMake, and vcpkg manifest mode**.
- GUI toolkit: **wxWidgets**.
- Numerical and FEM implementation: **custom C++**.
- Solver engine: independently runnable command-line application.
- Desktop application: tested Windows GUI that launches the solver as a separate process.
- Distribution: portable ZIP and offline Inno Setup installer workflows.
- Validation: automated tests, analytical benchmarks, GUI acceptance, package
  verification, portable clean-machine testing, and local installer acceptance.
- Remaining submission deliverable: record the planned 15-minute demonstration video.
- Important limitation: results must be independently validated before engineering use.

## Current capabilities

### Finite element analysis

- Linear-static solid mechanics with isotropic linear-elastic materials.
- Multiple materials in one model.
- Q4 plane-stress elements from ABAQUS `CPS4` input.
- Q4 plane-strain elements from ABAQUS `CPE4` input.
- H8 three-dimensional solid elements from ABAQUS `C3D8` input.
- Prescribed displacements, concentrated nodal loads, and element pressure loads.
- Direct elimination for displacement boundary conditions.
- Element-matrix assembly into COO storage, conversion to CSR, and solution with
  the Conjugate Gradient method.
- Displacement, reaction force, strain, stress, von Mises stress, and principal
  stress recovery.
- Consistent units supplied by the user; automatic unit conversion is not implemented.

### Input and output

- ABAQUS `.inp` parsing for supported nodes, elements, node/element sets,
  materials, solid sections, boundary conditions, concentrated loads, and pressures.
- Structured inspection of single-part, single-untransformed-instance Abaqus/CAE
  files, including scoped sets, separate element sets, element surfaces, and `*DSLOAD`.
- Inspection recognizes upcoming `CPS3`, `CPS4R`, `C3D4`, and `C3D8R` paths;
  their numerical solution remains Stage 2 work and is not claimed in version 0.1.0.
- ASCII `.vtu` result files for ParaView and other VTK-compatible visualizers.
- Versioned JSON analysis requests, lifecycle progress, completion summaries,
  and project files.
- Supplied Q4 plane-stress, Q4 plane-strain, H8 compression, cantilever,
  plate-with-a-hole, and multiple-material examples.

### Windows workbench

- Inspect a supported ABAQUS model before solving it.
- Create, save, reopen, and recover file-based FinEleMethod projects.
- Launch one solver process asynchronously and display validated progress.
- Request cooperative cancellation without forcibly terminating the solver.
- Store immutable input snapshots and results in numbered `run-0001`,
  `run-0002`, and later directories.
- Refresh, select, validate, and reopen completed run history.
- Open VTU results through the Windows-associated application.
- Preserve user projects independently from application installation and removal.

## Validation status

- **511 C++ tests:** passing in the latest local Release test run.
- **Q4 cantilever benchmark:** compared with beam theory using the documented mesh tolerance.
- **Q4 plate-with-a-hole benchmark:** compared with the Kirsch hoop-stress solution.
- **H8 compression benchmark:** compared with the analytical uniaxial solution.
- **GUI acceptance:** completed through the maintained interactive checklist.
- **Portable clean-machine check:** passed on a separate Windows laptop.
- **Installer acceptance:** install, launch, solve, shortcuts, Windows registration,
  uninstall, and user-project preservation passed on the development computer.
- **Windows CI:** builds, analyzes, tests, stages, verifies, and publishes the ZIP,
  manifest, installer, and their checksums.
- **Known installer gap:** the bundled VC++ runtime installation branch still needs
  a clean-machine test where no equal or newer runtime is already installed.

Detailed evidence and exact qualifications are maintained in
[Release Validation](docs/RELEASE_VALIDATION.md) and
[GUI Acceptance](docs/GUI_ACCEPTANCE.md).

## Downloadable application

- The portable ZIP requires the Microsoft Visual C++ x64 Runtime to be installed separately.
- The offline installer bundles the matching Microsoft Visual C++ x64 Redistributable.
- End users do not need Visual Studio, CMake, Git, vcpkg, or Inno Setup.
- ParaView remains a separate download for result visualization.
- The current installer is not code-signed, so Windows may show an unknown-publisher warning.
- The published [FinEleMethod 0.1.0 release](https://github.com/AnoopPatel582/FinEleMethod/releases/tag/v0.1.0)
  provides the installer, portable ZIP, matching checksums, and release manifest.
- Successful Windows CI runs also retain downloadable build files in the run's
  **Artifacts** section.

For end-user instructions, read:

- [Windows ZIP Quick Start](docs/WINDOWS_QUICK_START.md)
- [Windows Installer](docs/INSTALLER.md)
- [Beginner's Guide](docs/BEGINNERS_GUIDE.md)

## Source-build prerequisites

- Windows x64.
- Visual Studio Community 2026 with **Desktop development with C++**.
- CMake and vcpkg components installed through Visual Studio.
- Git.
- Inno Setup 6 only when building the installer.

## Configure, build, and test

Open **Developer PowerShell for Visual Studio 2026** in the repository:

```powershell
$env:VCPKG_ROOT = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\vcpkg"
cmake --preset windows-msvc
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
```

Build and test the optimized Release configuration:

```powershell
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release
```

Additional quality checks:

```powershell
.\cmake\CheckFormatting.ps1
.\cmake\CheckClangTidy.ps1
cmake --preset windows-msvc-analysis
cmake --build --preset windows-msvc-analysis
```

## Stage and package the application

Stage the Release application:

```powershell
cmake --install .\out\build\windows-msvc `
  --config Release `
  --prefix .\out\install\windows-msvc-release
```

Verify the staged solver, GUI, examples, documentation, and 12 app-local DLLs:

```powershell
.\cmake\VerifyStagedApplication.ps1 `
  -StageDirectory .\out\install\windows-msvc-release `
  -ReleaseDirectory .\out\build\windows-msvc\Release
```

Create the portable ZIP and checksum:

```powershell
.\cmake\CreateWindowsArchive.ps1 `
  -StageDirectory .\out\install\windows-msvc-release `
  -OutputFile .\out\package\FinEleMethod-windows-x64.zip
```

Create the offline Windows installer and checksum:

```powershell
.\cmake\BuildWindowsInstaller.ps1
```

## Run FinEleMethod

Launch the development workbench:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethodGui.exe
```

Display version and build diagnostics:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe --build-info
```

Inspect a model without solving:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --inspect .\examples\abaqus\q4_tension.inp
```

Solve a model and write a ParaView result:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\q4_tension.inp `
  --output .\out\q4_tension.vtu
```

Supported verification and benchmark inputs:

- `examples\abaqus\q4_tension.inp`
- `examples\abaqus\q4_plane_strain_tension.inp`
- `examples\abaqus\h8_compression.inp`
- `examples\abaqus\q4_cantilever.inp`
- `examples\abaqus\q4_plate_with_hole.inp`
- `examples\abaqus\q4_multiple_materials.inp`

## Architecture and documentation

- [Architecture](docs/ARCHITECTURE.md): component boundaries and data flow.
- [Project Decisions](docs/PROJECT_DECISIONS.md): confirmed scope and design decisions.
- [Q4 Formulation](docs/formulations/Q4.md): plane-stress and plane-strain mathematics.
- [H8 Formulation](docs/formulations/H8.md): three-dimensional element mathematics.
- [System Solution](docs/formulations/SYSTEM_SOLUTION.md): assembly, constraints,
  CSR storage, Conjugate Gradient, and reaction recovery.
- [Benchmarks](docs/benchmarks): analytical comparisons and tolerances.
- [Release Validation](docs/RELEASE_VALIDATION.md): package and acceptance evidence.
- [Demonstration Plan](docs/DEMONSTRATION.md): timed 15-minute video sequence.

The implemented numerical flow is:

```text
ABAQUS input -> validated model -> element matrices -> COO assembly -> CSR
-> direct displacement elimination -> Conjugate Gradient solve
-> displacement/reaction/stress/strain recovery -> VTU and JSON results
```

Build the strict documentation site with:

```powershell
python -m pip install -r .\docs\requirements.txt
mkdocs build --strict
```

- Generated site: `out\docs-site\index.html`.
- Generated C++ API reference: run `.\cmake\BuildApiDocumentation.ps1` and open
  `out\docs\api\html\index.html`.

## Licence status

- No open-source licence has been granted at this stage.
- All rights are reserved.
