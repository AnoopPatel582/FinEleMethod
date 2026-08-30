#pragma once

#include "finelemethod/elements/h8_jacobian.hpp"
#include "finelemethod/math/dense_matrix.hpp"

namespace finelemethod::elements
{
struct H8PhysicalDerivatives
{
    // Rows 0, 1, and 2 contain dN/dx, dN/dy, and dN/dz, respectively.
    math::DenseMatrix derivatives;
    double jacobian_determinant;
};

[[nodiscard]] H8PhysicalDerivatives h8_shape_function_physical_derivatives(
    const H8NodeCoordinates &coordinates, double xi, double eta, double zeta);
} // namespace finelemethod::elements
