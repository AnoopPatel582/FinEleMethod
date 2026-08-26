#pragma once

#include "finelemethod/elements/q4_jacobian.hpp"
#include "finelemethod/math/dense_matrix.hpp"

namespace finelemethod::elements
{
struct Q4PhysicalDerivatives
{
    // Row 0 contains dN/dx and row 1 contains dN/dy.
    math::DenseMatrix derivatives;
    double jacobian_determinant;
};

[[nodiscard]] Q4PhysicalDerivatives q4_shape_function_physical_derivatives(
    const Q4NodeCoordinates &coordinates, double xi, double eta);
} // namespace finelemethod::elements
