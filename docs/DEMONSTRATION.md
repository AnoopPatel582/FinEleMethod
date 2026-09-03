# Fifteen-minute demonstration plan

This is a rehearsal and recording guide. It is not the required video itself,
and it does not claim that the GUI acceptance cases have been completed.

## Prepare before recording

- Choose the exact build and keep its commit, successful CI run, and ZIP checksum.
- Complete the relevant [GUI acceptance cases](GUI_ACCEPTANCE.md) on that build.
- Prepare separate disposable Q4 plane-stress, plane-strain, and H8 projects.
- Have ParaView and the documentation open; confirm result loading works.
- Run [package validation](RELEASE_VALIDATION.md) in advance and retain its logs.
- Use fresh project/run names for the recording. Do not record private files,
  account credentials, or unrelated personal windows.
- Rehearse the timing. Do not present a precomputed result as a live solve.

## Recording sequence

| Time | Demonstrate | Explain |
| --- | --- | --- |
| 0:00-1:00 | Project title and scope | Custom C++20 Windows linear-static solver, Q4 and H8, consistent units |
| 1:00-3:00 | Architecture diagram and selected source files | Element matrices, COO assembly, direct displacement elimination, CSR and Conjugate Gradient; GUI launches the CLI separately |
| 3:00-6:00 | Create/run Q4 plane-stress project; open VTU in ParaView | Input inspection, immutable run directory, displacement/stress fields, deformation scale versus physical displacement |
| 6:00-8:00 | Plane-strain and H8 runs | Formulation differences, pressure loading, axial compression and Poisson expansion |
| 8:00-10:00 | Three benchmark reports | Analytical comparison, tolerances, mesh-dependent error, limitations of low-order elements |
| 10:00-12:00 | Reopen history and recover a metadata snapshot | Saved runs, unsaved-recovery marker, confirmation before leaving; snapshots do not restore model contents |
| 12:00-14:00 | Test/CI evidence and extracted ZIP layout | Automated numerical and package checks, build diagnostics, setup without developer tools |
| 14:00-15:00 | Remaining work and conclusion | Clean-machine qualification, interactive checks, future unit conversion/editor/embedded visualization |

## Benchmark talking points

- [Cantilever](benchmarks/cantilever_beam.md): explain the recorded `9.04%`
  displacement error and `10%` test limit. Passing the limit is not exact beam
  agreement; the fully integrated Q4 mesh is stiff in bending.
- [Plate with a hole](benchmarks/plate_with_hole.md): explain the recorded `1.91%`
  hoop-stress error at a Gauss point. Do not compare that interior value directly
  with the boundary stress-concentration factor of three.
- [H8 compression](benchmarks/h8_compression.md): show the expected `-0.01`
  top-face Z displacement, `-10` axial stress, and `+10` total support reaction.

The percentages above are the maintained report values. If rerunning changes
them, investigate and update the evidence instead of narrating an old number.

## If a live action fails

Keep the exact error and identify the affected workflow. Use clearly labelled
previously validated output only if needed to continue explaining the solver.
Do not describe a blocked or skipped check as passed. Fix and rehearse the
workflow before making the final recording intended for evaluation.
