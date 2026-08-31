# H8 block-compression benchmark

This benchmark validates the three-dimensional H8 workflow against the exact
uniaxial linear-elastic solution for a unit cube under uniform compression.

## Model

- Dimensions: `1 x 1 x 1`
- Young's modulus: `1000`
- Poisson's ratio: `0.25`
- Mesh: one fully integrated H8 element
- Load: pressure `10` acting inward on the top face
- Boundary conditions: Z displacement fixed on the bottom face, X displacement
  fixed on the X=0 face, and Y displacement fixed on the Y=0 face

The constraints prevent rigid-body motion while allowing the cube to expand
laterally through the Poisson effect. The runnable input is
[`examples/abaqus/h8_compression.inp`](../../examples/abaqus/h8_compression.inp).

## Comparison

For uniaxial stress, the exact axial strain is
`epsilon_z = sigma_z/E = -10/1000 = -0.01`. The two transverse strains are
`epsilon_x = epsilon_y = -nu * epsilon_z = 0.0025`.

FinEleMethod reproduces, within numerical precision:

- top-face Z displacement: `-0.01`
- transverse X and Y strains: `0.0025`
- axial stress: `-10.0`
- von Mises stress: `10.0`
- principal stresses: `[0.0, 0.0, -10.0]`
- total bottom-face Z reaction: `10.0`

The automated benchmark reads and solves the checked-in ABAQUS input file and
checks these values at all eight Gauss points with a tolerance of `1e-11`.
