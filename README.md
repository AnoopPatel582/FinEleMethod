# FinEleMethod

FinEleMethod is a Windows finite element solver being developed from scratch in
C++20. A tested command-line solver engine and a native Windows engineering
workbench are available; the solver remains independently runnable.

The project is under active development. Its command-line engine can currently
solve supported ABAQUS Q4 plane-stress, Q4 plane-strain, and H8 three-dimensional
models and write ParaView results.

A native wxWidgets Windows workbench shell is also available. It establishes the
GUI application boundary while the solver remains an independent command-line
process. The current shell can select an existing ABAQUS `.inp` model and display
its path, create or reopen a file-based FinEleMethod project, and launch the
command-line solver asynchronously for one analysis at a time. Validated
solver lifecycle messages are displayed while the analysis is running. After a
validated completion summary is received, model and solver statistics are shown
and the VTU result can be opened with the Windows-associated application. The
project area also reports the number of prepared analysis runs and the latest
run identifier. Its run selector lists the numbered history and displays the
directory of a selected run. Selecting a completed run validates and restores
its summary and makes its existing VTU result available to open again. The
selected run directory can also be opened directly in Windows Explorer.
Run-history entries use recorded lifecycle states when available. Entries are
labelled `completed` only when their versioned summary and referenced VTU result
pass the completion checks. Missing or invalid lifecycle state falls back to
`completed` or `not completed`; an old `executing` label does not prove that its
process is still running.
The history can be refreshed from the workbench or Analysis menu to discover
run directories and completed outputs changed outside the application.
If reading the refreshed history fails, the last successfully loaded list and
selection remain available; the workbench reports the refresh error.
If history refresh fails as an analysis finishes, the controls are restored and
the solver outcome is still validated. The outcome dialog includes a separate
history warning; the history area indicates stale entries until Refresh succeeds.
An open project can be saved through **File → Save Project** or `Ctrl+S`; this
uses the atomic project save, retains one backup, and removes a superseded
autosave snapshot.

**File → Create Recovery Snapshot** writes a separate metadata snapshot after
validating the open project. Replacing an existing snapshot requires confirmation.
**File → Recover Project Autosave...** validates that snapshot independently of
the main JSON and loads it only after confirmation. **Save Project** then commits
the recovered metadata and retains the previous main JSON as a backup.
Select the `.autosave.json` directly if the main JSON is missing; saving recreates
the main JSON without replacing any existing backup. If the save succeeds but
snapshot cleanup fails, the workbench reports a cleanup warning rather than a
save failure. A failed save leaves the snapshot untouched.
Snapshots do not copy or restore ABAQUS model contents or result files. This is
an explicit snapshot workflow, not timed autosaving of model edits.

The project-storage layer can create a new file-based project, copy its
authoritative ABAQUS model into `input/`, create an initially empty `runs/`
directory, and write the versioned `<ProjectName>.json` project file.
Existing project files can be reopened after their schema, identity, relative
input path, authoritative model, and run directory are validated.
Project saves atomically replace the authoritative JSON while retaining its
previous contents as `<ProjectName>.json.bak`. Autosave snapshots use the
separate `<ProjectName>.autosave.json` file and can be validated or removed
without changing either authoritative file.
Save and autosave validate the referenced input file and canonical `runs/`
directory before replacing any stored metadata.
Each prepared analysis receives the next numbered directory (`run-0001`,
`run-0002`, and so on), an immutable model snapshot, a validated request, and a
dedicated results directory. Its versioned `analysis-state.json` atomically
records the latest preparing, executing, result-writing, completed, failed, or
cancelled lifecycle event. The project layer can rediscover these runs in
ascending numerical order, providing the storage foundation for workbench run
history.
Cooperative analysis cancellation uses a run-local
`cancellation-requested.flag` rather than forcibly terminating the solver. In
request mode, the CLI checks this flag at safe lifecycle boundaries, reports the
`cancelled` state, and exits with code `6` without writing completion outputs.
The workbench exposes the same mechanism through **Cancel Analysis** and reports
cancellation separately from solver failure.

## Current capabilities

- C++20 command-line executable built with MSVC and CMake.
- Reproducible dependencies managed with a vcpkg manifest.
- Automated tests using GoogleTest and CTest.
- ABAQUS `.inp` parsing for nodes, CPS4, CPE4, and C3D8 elements, isotropic
  materials, solid sections, nodal displacement constraints, and concentrated
  nodal loads.
- Sparse linear-static Q4 plane-stress, Q4 plane-strain, and H8 solutions with
  displacement, reaction, strain, stress, von Mises, and principal-stress
  recovery.
- ASCII `.vtu` output for ParaView.
- Optional versioned JSON analysis summaries for application integration.
- Reusable validated model summaries for Q4 and H8 workbench inspection.
- Workbench model inspection showing formulation, model sizes, constraints, and loads before solve.

## Prerequisites

- Windows.
- Visual Studio Community 2026 with the **Desktop development with C++**
  workload.
- CMake and vcpkg components installed through Visual Studio.
- Git.

The supplied CMake preset uses the `VCPKG_ROOT` environment variable rather
than a computer-specific absolute path.

## Configure

Open **Developer PowerShell for Visual Studio 2026**, move to the repository,
and set `VCPKG_ROOT` for the current terminal session:

```powershell
$env:VCPKG_ROOT = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\vcpkg"
cmake --preset windows-msvc
```

Validate and inspect a supported ABAQUS model without solving it:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe --inspect .\examples\abaqus\q4_tension.inp
```

Create optimized Windows binaries with the Release preset:

```powershell
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release
```

Stage a distributable application folder after the Release build:

```powershell
cmake --install .\out\build\windows-msvc `
  --config Release `
  --prefix .\out\install\windows-msvc-release
```

The staged folder contains the command-line solver, workbench, required
app-local runtime DLLs, documentation, and ABAQUS examples. Runtime staging
copies the complete DLL set placed beside the Release applications by vcpkg so
transitive dependencies such as `z.dll` are not omitted. It intentionally
excludes test binaries and development libraries. The Microsoft Visual C++
Runtime prerequisite will be handled by the future installer.

Windows CI repeats this staging workflow and verifies the complete DLL set,
command-line model inspection, and workbench startup with development paths
removed from the process environment.

Check every tracked C++ source and header against `.clang-format`:

```powershell
.\cmake\CheckFormatting.ps1
```

After configuring the MSVC build, run the approved static analysis rules across
all production source files:

```powershell
.\cmake\CheckClangTidy.ps1
```

Run MSVC native code analysis as an independent warning-as-error build:

```powershell
cmake --preset windows-msvc-analysis
cmake --build --preset windows-msvc-analysis
```

Generate the searchable C++ API reference with the pinned Doxygen tool:

```powershell
.\cmake\BuildApiDocumentation.ps1
```

Open `out/docs/api/html/index.html` in a browser. The helper downloads Doxygen
1.18.0 only when it is unavailable locally and verifies the official SHA-256
checksum before use.

Create a ZIP archive from the verified staged folder:

```powershell
.\cmake\CreateWindowsArchive.ps1 `
  -StageDirectory .\out\install\windows-msvc-release `
  -OutputFile .\out\package\FinEleMethod-windows-x64.zip
```

The version-neutral archive name is temporary until the first formal release
version is selected. Packaging also creates
`FinEleMethod-windows-x64.zip.sha256`; verify it with:

```powershell
.\cmake\VerifyWindowsArchiveChecksum.ps1 `
  -ArchiveFile .\out\package\FinEleMethod-windows-x64.zip `
  -ChecksumFile .\out\package\FinEleMethod-windows-x64.zip.sha256
```

After a successful Windows CI run, the same verified ZIP is available in the
workflow run's **Artifacts** section as `FinEleMethod-windows-x64.zip`.

If Visual Studio or vcpkg is installed elsewhere, set `VCPKG_ROOT` to the
directory containing `vcpkg.exe` and the `scripts` directory.

The first configuration downloads and builds the dependencies declared in
`vcpkg.json`. Later configurations reuse the local package cache.

## Build

```powershell
cmake --build --preset windows-msvc-debug
```

## Test

```powershell
ctest --preset windows-msvc-debug
```

GoogleTest discovery allows up to 60 seconds for the newly linked test executable
to start and enumerate its tests on Windows CI. This replaces CMake's five-second
default after a hosted-runner discovery timeout; it does not skip tests or relax
their assertions. A discovery timeout is reported during the build, before the
CTest execution step.

## Run

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe --help
```

Launch the current Windows workbench shell with:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethodGui.exe
```

Run the supplied Q4 uniaxial-tension verification model from the repository
root:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\q4_tension.inp `
  --output .\out\q4_tension.vtu
```

Open `out\q4_tension.vtu` in ParaView. The analytical solution has a right-edge
X displacement of `0.01`, X reactions of `-5.0` at nodes 1 and 4, and uniform
X stress and von Mises stress of `10.0`.

Run the supplied Q4 plane-strain verification model:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\q4_plane_strain_tension.inp `
  --output .\out\q4_plane_strain_tension.vtu
```

Its analytical solution has a right-edge X displacement of `0.009375`, a
top-edge Y displacement of `-0.003125`, uniform X stress of `10.0`, and
constrained out-of-plane stress of `2.5`.

Run the supplied H8 three-dimensional block-compression verification model:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\h8_compression.inp `
  --output .\out\h8_compression.vtu
```

Its analytical solution has a top-face Z displacement of `-0.01`, transverse
X and Y strains of `0.0025`, uniform Z stress of `-10.0`, von Mises stress of
`10.0`, and a total bottom-face Z reaction of `10.0`. See
[the H8 compression benchmark report](docs/benchmarks/h8_compression.md).

Run the supplied Q4 cantilever-beam benchmark:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\q4_cantilever.inp `
  --output .\out\q4_cantilever.vtu
```

The automated benchmark compares its tip displacement and support reaction
with beam theory. See [the cantilever benchmark report](docs/benchmarks/cantilever_beam.md).

Run the supplied Q4 plate-with-a-circular-hole benchmark:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\q4_plate_with_hole.inp `
  --output .\out\q4_plate_with_hole.vtu
```

The automated benchmark compares the recovered hoop stress near the hole with
the analytical Kirsch solution. See
[the plate-with-a-hole benchmark report](docs/benchmarks/plate_with_hole.md).

Run the supplied Q4 multiple-material verification model:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\q4_multiple_materials.inp `
  --output .\out\q4_multiple_materials.vtu
```

Its two elements carry the same axial stress but use elastic moduli of `1000`
and `2000`, producing exact axial strains of `0.01` and `0.005`, respectively.

Add `--summary` to an ABAQUS analysis when a machine-readable completion record
is needed by another application:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --input .\examples\abaqus\q4_tension.inp `
  --output .\out\q4_tension.vtu `
  --summary .\out\q4_tension-summary.json
```

The JSON document uses `protocolVersion` `1` and records the completion status,
analysis type, input and result paths, model sizes, solver iteration count, and
final residual norm. Application integrations validate this summary before
accepting a run as successfully completed.

Add `--json-progress` after the other ABAQUS options to make standard output a
JSON Lines stream for application integration. Each line reports a versioned
analysis lifecycle event; human-readable errors remain on standard error. The
integration layer validates each incoming record before using its state or
message.

The GUI integration layer can write the same validated `analysis-request.json`
used by the CLI. Its version-1 format is:

```json
{
  "protocolVersion": 1,
  "inputFile": "input/model.inp",
  "resultFile": "results/model.vtu",
  "summaryFile": "results/analysis-summary.json"
}
```

All paths are relative to the request file's directory. Run it with:

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe `
  --request .\path\to\analysis-request.json
```

Request mode writes JSON Lines progress automatically and creates both the VTU
result and JSON summary at the requested locations. Parent directories must
already exist.

The ABAQUS reader accepts direct node IDs and explicit-list or `GENERATE`
`*NSET` names in both `*BOUNDARY` and `*CLOAD`. Uniform Q4 edge pressures use
`*DLOAD` with `P1` through `P4` and direct element IDs or element-set names.
H8 face pressures use `*DLOAD` with `P1` through `P6` and direct element IDs or
element-set names. Other element types will be added in later increments.

## Project decisions

Read [docs/PROJECT_DECISIONS.md](docs/PROJECT_DECISIONS.md) before proposing
architectural or implementation changes. It records the confirmed scope,
toolchain, FEM roadmap, numerical approach, storage model, and future GUI
protocol.

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for the implemented component
architecture, solver workflow, numerical data flow, and future GUI boundary.
The [Q4 formulation](docs/formulations/Q4.md) documents the implemented plane-
stress and plane-strain mathematics. The [H8 formulation](docs/formulations/H8.md)
documents the implemented three-dimensional solid mechanics.
The [global-system formulation](docs/formulations/SYSTEM_SOLUTION.md) explains
COO assembly, displacement elimination, CSR storage, Conjugate Gradient, and
reaction recovery.

New users can follow the complete Windows setup and first-analysis workflow in
the [Beginner's Guide](docs/BEGINNERS_GUIDE.md).

The [GUI acceptance checklist](docs/GUI_ACCEPTANCE.md) records interactive
verification separately from automated tests and tracks remaining release gates.

## Build the documentation site

Install the pinned documentation tool and build the strict MkDocs site:

```powershell
python -m pip install -r .\docs\requirements.txt
mkdocs build --strict
```

The generated site is written to `out\docs-site\index.html`.

## Licence status

No open-source licence has been granted at this stage. All rights are reserved.
