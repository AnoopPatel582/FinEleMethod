# Q4 cantilever-beam benchmark

This benchmark validates the Q4 plane-stress workflow against the classical
Euler-Bernoulli cantilever solution.

## Model

- Length: `10`
- Height: `1`
- Thickness: `1`
- Young's modulus: `1000`
- Poisson's ratio: `0.25`
- Mesh: `20 x 4` fully integrated Q4 elements
- Boundary condition: all left-edge X and Y displacements fixed
- Load: total downward end load `1`, distributed parabolically over the free
  edge nodes

The runnable input is
[`examples/abaqus/q4_cantilever.inp`](../../examples/abaqus/q4_cantilever.inp).

## Comparison

For a rectangular section, `I = bh^3/12 = 1/12`. Euler-Bernoulli theory gives

`v_tip = -PL^3/(3EI) = -4.0`.

FinEleMethod predicts `-3.63828` at the free-end neutral-axis node. The relative
error is `9.04%`. The automated acceptance limit is `10%` and the summed
vertical support reaction must equal `1` within numerical precision.

This is a discretization benchmark, not an exact constant-strain patch test.
Fully integrated low-order Q4 elements are artificially stiff in slender
bending, so the result should converge toward beam theory as the mesh and
formulation are improved.
