#pragma once

#include "finelemethod/math/dense_matrix.hpp"

#include <array>

namespace finelemethod::elements
{
using H8NodeCoordinates = std::array<std::array<double, 3>, 8>;

struct H8Jacobian
{
    math::DenseMatrix matrix;
    double determinant;
    math::DenseMatrix inverse;
};

// Coordinates follow the standard H8 natural node order documented by
// h8_shape_functions.
[[nodiscard]] H8Jacobian h8_jacobian(const H8NodeCoordinates &coordinates, double xi, double eta,
                                     double zeta);
} // namespace finelemethod::elements
