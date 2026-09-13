# ADR 0002: Abaqus/CAE Import Profile

## Status

Accepted for post-0.1 development.

## Context

FinEleMethod 0.1 parses a deliberately small, flat ABAQUS input subset for
`CPS4`, `CPE4`, and `C3D8`. External verification models supplied as ordinary
Abaqus/CAE exports use part and assembly namespaces, instance-qualified sets,
separate element-set definitions, element-based surfaces, `*DSLOAD`, default
section data, and the additional element types `CPS3`, `CPS4R`, `C3D4`, and
`C3D8R`.

Editing those external files would weaken the import test and could change the
model being verified. Treating a reduced-integration element as its
full-integration counterpart would also misrepresent its formulation.

## Decision

- Preserve the original input files and expand FinEleMethod instead.
- Add a structured importer that resolves ABAQUS scopes into canonical nodes,
  elements, material assignments, prescribed displacements, point loads, and
  element-face pressures.
- The first CAE profile supports one part and one untransformed instance. It
  rejects broader assembly structures explicitly.
- Recognize `CPS3`, `CPS4R`, `C3D4`, and `C3D8R` during inspection before their
  numerical formulations are enabled for analysis.
- Keep `CPS4`, `CPE4`, and `C3D8` behavior and tests intact.
- Implement each new element as a genuine formulation. In particular,
  reduced-integration elements must not be aliases for full integration.

## Consequences

- The workbench and `--inspect` can validate and summarize the supplied CAE
  models without rewriting them.
- Solver dispatch continues to reject recognized new types until their Stage 2
  numerical implementations are complete.
- Full Abaqus language compatibility is not claimed. Multiple parts, multiple
  instances, transformed instances, nonlinear steps, and unsupported keywords
  remain outside this import profile.
- Stage 2 can consume one resolved representation rather than duplicating
  assembly and surface resolution in every element parser.
