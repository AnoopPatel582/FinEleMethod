# FinEleMethod Project Decisions

This document is the permanent source of truth for decisions confirmed during
the initial FinEleMethod planning discussions. Future development conversations
must read this document before proposing architecture or implementation changes.

## 1. Product vision

- Product and software-development repository name: **FinEleMethod**.
- Build a finite element solver from scratch as a serious software-engineering
  and learning project, not only as an academic submission.
- Develop feature by feature, test each feature properly, and push progress to
  GitHub incrementally.
- The source repository is public for now; its visibility may change later.
- The finished Windows application is intended for demonstrations, a future FEM
  website, resumes and recruitment, and potentially practical public use.
- No open-source license is selected at present. Copyright remains reserved.
  A separate end-user licence agreement may be created for distributed binaries
  later.

## 2. Supported platform and toolchain

- Target platform: **Windows only**.
- Programming language standard: **C++20**.
- Compiler: **Microsoft Visual C++ (MSVC)**.
- Build system: **CMake**.
- Primary development environment: **Visual Studio**.
- Dependency management: **vcpkg manifest mode**.
- Unit-testing framework: **GoogleTest**.
- Logging library: **spdlog**.
- JSON library: **nlohmann/json**.
- Code-quality tools: **clang-format**, **clang-tidy**, and MSVC `/analyze`.
- Continuous integration: **GitHub Actions**.
- Git workflow: **GitHub Flow**.

## 3. Development strategy

1. Build and validate a command-line solver engine first.
2. Add features in small, testable increments.
3. Keep a cohesive feature and its regression tests together.
4. Commit and push verified improvements incrementally.
5. Build the Windows GUI after the corresponding command-line capability works.

The FEM formulations and numerical calculations will be custom implementations.
General-purpose infrastructure libraries may be used where already approved.

## 4. Initial solver scope

- Linear static solid mechanics.
- Isotropic linear-elastic materials.
- Multiple materials in one model.
- ABAQUS `.inp` model input.
- Displacement boundary conditions.
- Point loads, followed later by edge pressure and face pressure loads.
- VTK XML unstructured-grid (`.vtu`) output in ASCII form initially.
- Results: displacements, strains, stresses, von Mises stress, principal stress,
  and reactions.
- Unit handling is initially based on consistent user-supplied units. Automatic
  unit conversion is future work.

## 5. Element implementation order

1. Q4 quadrilateral element for two-dimensional analysis.
2. Q4 plane-stress formulation and validation.
3. Q4 plane-strain formulation.
4. H8 hexahedral element for three-dimensional analysis.

## 6. Numerical architecture

- Begin with a custom dense-matrix implementation for foundational learning.
- Form element matrices independently.
- Assemble the global system in COO sparse format.
- Convert COO to CSR format.
- Apply displacement boundary conditions using direct elimination.
- Implement Gaussian elimination with partial pivoting as the first solver.
- Add a sparse Conjugate Gradient solver later.
- Use the 3D engineering-strain order
  `[epsilon_x, epsilon_y, epsilon_z, gamma_xy, gamma_yz, gamma_zx]` and the
  corresponding stress order `[sigma_x, sigma_y, sigma_z, tau_xy, tau_yz,
  tau_zx]` throughout the H8 formulation and result pipeline.

## 7. Required validation problems

- Cantilever beam.
- Plate with a hole.
- Three-dimensional block under compression.

Solver results will be compared with analytical or established benchmark
solutions where appropriate.

## 8. Project and run storage

- Storage is file-based initially; database or other storage can be reconsidered
  later.
- A project is stored in a project directory.
- The main project file is JSON and follows the naming convention
  `<ProjectName>/<ProjectName>.json`.
- The project JSON contains a `schemaVersion` and uses relative paths.
- Project saves are atomic and retain one backup copy.
- Autosave uses a separate `.autosave.json` file.
- An imported ABAQUS `.inp` file is copied into the project's input directory.
- The copied `.inp` file is initially the authoritative model source.
- The initial GUI may inspect imported model data; model editing is later work.
- Each analysis creates a numbered, immutable run directory.

## 9. Command-line and GUI relationship

- The solver remains a standalone command-line executable.
- The future GUI will be a Windows engineering workbench built with **wxWidgets**.
- wxWidgets will be managed through vcpkg and CMake and compiled with MSVC.
- The GUI launches the CLI solver as a separate process.
- Analyses run in the background, with one active analysis at a time initially.
- ParaView is the first visualization workflow. Embedded VTK visualization is
  later work.

## 10. Analysis-process protocol

- The GUI writes `analysis-request.json`.
- Analysis-request protocol version 1 requires relative `inputFile`,
  `resultFile`, and `summaryFile` paths, resolved from the request directory.
- The CLI reports progress through JSON Lines on standard output.
- Progress records use event type `analysis-progress` and the lifecycle states
  `preparing`, `executing`, `writing-results`, `completed`, `failed`, and
  `cancelled`.
- Results and a summary are written to files in the run directory.
- The protocol includes a `protocolVersion` for compatibility management.
- The initial analysis-summary contract uses `protocolVersion` `1`.
- Cancellation uses a cancellation-flag mechanism.
- Run lifecycle states will distinguish preparation, execution, completion,
  failure, and cancellation.
- Stable process exit codes:

| Code | Meaning |
| ---: | --- |
| 0 | Success |
| 1 | Command-line usage error |
| 2 | Input parsing error |
| 3 | Model validation error |
| 4 | Numerical solution error |
| 5 | Result-writing error |
| 6 | Cancelled |
| 10 | Unexpected internal error |

## 11. Documentation strategy

- Markdown for maintained project documentation.
- MkDocs for a browsable documentation site.
- Mermaid for architecture and workflow diagrams.
- Doxygen for source-code API documentation.
- Architecture Decision Records (ADRs) for important future decisions and
  changes.
- Final project material will include architecture, flowcharts, formulations,
  test inputs, benchmark results, a beginner's guide, and a demonstration video.

## 12. Change-control rule

When a confirmed decision changes, do not silently rewrite history. Create an
ADR explaining the context, selected option, consequences, and the decision it
supersedes, then update this summary to point to that ADR.
