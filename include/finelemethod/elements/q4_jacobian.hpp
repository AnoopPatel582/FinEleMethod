#pragma once

#include "finelemethod/math/dense_matrix.hpp"

#include <array>

namespace finelemethod::elements
{
using Q4NodeCoordinates = std::array<std::array<double, 2>, 4>;

struct Q4Jacobian
{
    math::DenseMatrix matrix;
    double determinant;
    math::DenseMatrix inverse;
};

// Coordinates follow the Q4 node order: (-1,-1), (1,-1), (1,1), (-1,1).
[[nodiscard]] Q4Jacobian q4_jacobian(const Q4NodeCoordinates &coordinates, double xi, double eta);
} // namespace finelemethod::elements
