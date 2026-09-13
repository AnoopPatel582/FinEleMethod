# Instructor Model Compatibility

The four instructor-provided Abaqus/CAE files were kept unchanged and executed
through the command-line solver on the Stage 3 development branch. Each run
completed global COO assembly, direct displacement elimination, CSR conversion,
Conjugate Gradient solution, result recovery, and VTU writing.

## Acceptance results

| File | Element | Nodes | Elements | CG iterations | Residual norm | Maximum displacement |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| `Q4.inp` | `CPS4R` | 2,359 | 2,254 | 866 | $2.13\times10^{-10}$ | $1.4555\times10^{-4}$ |
| `T3.inp` | `CPS3` | 2,359 | 4,508 | 961 | $1.96\times10^{-10}$ | $1.4537\times10^{-4}$ |
| `T4.inp` | `C3D4` | 1,582 | 6,824 | 673 | $1.09\times10^{-9}$ | $1.4394\times10^{-4}$ |
| `H8.inp` | `C3D8R` | 2,860 | 1,956 | 857 | $6.19\times10^{-10}$ | $1.4569\times10^{-4}$ |

## Equilibrium check

Summing the recovered nodal reaction vectors produced:

| Pair | Total reaction |
| --- | --- |
| `Q4.inp`, `T3.inp` | $F_y=-10$ within floating-point roundoff |
| `T4.inp`, `H8.inp` | $F_y=-50$ within floating-point roundoff |

The two alternative 2D meshes agree closely on maximum displacement, as do
the two alternative 3D meshes. This is a useful cross-discretization check; it
does not claim numerical identity with ABAQUS, particularly for proprietary
hourglass-control behavior.

All four result files were parsed as XML and checked for the expected number of
cell results. They contain displacement, reaction force, strain, stress, von
Mises stress, and three principal stresses.
