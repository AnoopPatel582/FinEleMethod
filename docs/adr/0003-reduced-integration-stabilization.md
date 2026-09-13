# ADR 0003: Stabilized reduced integration for CPS4R and C3D8R

- Status: Accepted
- Scope: Element stiffness kernels for instructor-provided reduced-integration
  models

## Context

One-point integration of bilinear Q4 and trilinear H8 displacement elements
passes a constant-strain patch test but introduces zero-energy hourglass
modes. An unstabilized global stiffness matrix can therefore be singular even
when the physical boundary conditions are sufficient.

ABAQUS hourglass control contains product-specific implementation details that
FinEleMethod does not reproduce. Silently treating an `R` element as a
full-integration element would also misrepresent the selected formulation.

## Decision

FinEleMethod forms the one-point stiffness $\mathbf{K}_{1pt}$ and stabilizes
only the stiffness omitted by one-point integration:

$$
\mathbf{K}_{R}=
\mathbf{K}_{1pt}+\alpha
\left(\mathbf{K}_{full}-\mathbf{K}_{1pt}\right),
\qquad \alpha=0.05.
$$

The default stabilization fraction is explicit in the public formulation API.
It must be finite and belong to $(0,1]$.

## Consequences

- Constant-strain energy remains identical to full integration because both
  rules integrate the constant-strain field exactly.
- Hourglass modes receive positive stiffness instead of remaining zero-energy
  mechanisms.
- The result is a transparent educational stabilization, not a claim of
  numerical identity with ABAQUS hourglass control.
- Tests separately verify the constant-strain patch behavior and the energy of
  a representative hourglass mode.
