# FinEleMethod Documentation

FinEleMethod is a custom C++20 finite element solver and Windows engineering
workbench for linear static solid mechanics.

The current solver supports:

- Q4 plane-stress and plane-strain elements.
- H8 three-dimensional solid elements.
- Isotropic linear-elastic materials, including multiple materials per model.
- ABAQUS `.inp` input with displacement, point, edge-pressure, and face-pressure loads.
- Sparse COO-to-CSR assembly and a custom Conjugate Gradient solver.
- ParaView `.vtu` results containing displacements, reactions, strains, stresses,
  von Mises stress, and principal stresses.

Start with the [Beginner's Guide](BEGINNERS_GUIDE.md) to build and run the
software. Read [Architecture](ARCHITECTURE.md) for the component and analysis
workflow, then use the formulation and benchmark pages for the mathematical and
validation details.
