#pragma once

#include "finelemethod/elements/q4_jacobian.hpp"
#include "finelemethod/math/dense_matrix.hpp"

namespace finelemethod::elements
{
struct Q4StrainDisplacement
{
    // Maps [u1, v1, ..., u4, v4] to [epsilon_x, epsilon_y, gamma_xy].
    math::DenseMatrix matrix;
    double jacobian_determinant;
};

[[nodiscard]] Q4StrainDisplacement q4_strain_displacement_matrix(
    const Q4NodeCoordinates &coordinates, double xi, double eta);
} // namespace finelemethod::elements
