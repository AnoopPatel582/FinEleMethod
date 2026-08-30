#pragma once

#include "finelemethod/elements/h8_jacobian.hpp"
#include "finelemethod/math/dense_matrix.hpp"

namespace finelemethod::elements
{
struct H8StrainDisplacement
{
    // Maps [u1, v1, w1, ..., u8, v8, w8] to the engineering strain order
    // [epsilon_x, epsilon_y, epsilon_z, gamma_xy, gamma_yz, gamma_zx].
    math::DenseMatrix matrix;
    double jacobian_determinant;
};

[[nodiscard]] H8StrainDisplacement h8_strain_displacement_matrix(
    const H8NodeCoordinates &coordinates, double xi, double eta, double zeta);
} // namespace finelemethod::elements
