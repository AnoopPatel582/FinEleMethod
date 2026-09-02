# Q4 plate-with-a-hole benchmark

This benchmark validates the Q4 plane-stress workflow against the analytical
Kirsch elasticity solution for an infinite plate containing a circular hole
under remote uniaxial tension.

## Model

- Hole radius: `1`
- Truncated outer radius: `10`
- Thickness: `1`
- Young's modulus: `1000`
- Poisson's ratio: `0.25`
- Remote X stress: `1`
- Mesh: `8 x 16` fully integrated Q4 elements with radial grading
- Symmetry: Y displacement fixed on the X axis and X displacement fixed on the
  Y axis
- Load: analytical Kirsch tractions applied on the outer boundary as consistent
  nodal forces

Only the first quadrant is modelled. The outer boundary is sufficiently far
from the hole to represent the infinite-domain solution, and applying its exact
tractions avoids prescribing an approximate uniform traction on a circular
truncation boundary.

The runnable input is
[`examples/abaqus/q4_plate_with_hole.inp`](https://github.com/AnoopPatel582/FinEleMethod/blob/main/examples/abaqus/q4_plate_with_hole.inp).

## Comparison

The automated test examines the Gauss point closest to the hole crown. At that
point, FinEleMethod predicts a hoop stress of `2.63094`; the Kirsch solution at
the same physical coordinates gives `2.58158`. The relative error is `1.91%`,
below the automated acceptance limit of `10%`.

The well-known stress concentration of `3` occurs exactly at the traction-free
hole boundary. Q4 stresses are recovered at interior Gauss points, so the test
compares stresses at the same interior location instead of incorrectly treating
a Gauss-point value as a boundary value.
