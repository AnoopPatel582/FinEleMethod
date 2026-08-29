# FinEleMethod

FinEleMethod is a Windows finite element solver being developed from scratch in
C++20. Development begins with a tested command-line solver engine; a Windows
engineering GUI will be added after the corresponding solver capabilities are
validated.

The project is under active development. Its command-line engine can currently
solve supported ABAQUS Q4 plane-stress models and write ParaView results.

## Current capabilities

- C++20 command-line executable built with MSVC and CMake.
- Reproducible dependencies managed with a vcpkg manifest.
- Automated tests using GoogleTest and CTest.
- ABAQUS `.inp` parsing for nodes, CPS4 elements, isotropic materials, solid
  sections, nodal displacement constraints, and concentrated nodal loads.
- Dense linear-static Q4 plane-stress solution with displacement, reaction,
  strain, stress, von Mises, and principal-stress recovery.
- ASCII `.vtu` output for ParaView.

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

## Run

```powershell
.\out\build\windows-msvc\Debug\FinEleMethod.exe --help
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

The ABAQUS reader accepts direct node IDs and explicit-list `*NSET` names in
both `*BOUNDARY` and `*CLOAD`. Generated node sets and other element or analysis
types will be added in later increments.

## Project decisions

Read [docs/PROJECT_DECISIONS.md](docs/PROJECT_DECISIONS.md) before proposing
architectural or implementation changes. It records the confirmed scope,
toolchain, FEM roadmap, numerical approach, storage model, and future GUI
protocol.

## Licence status

No open-source licence has been granted at this stage. All rights are reserved.
