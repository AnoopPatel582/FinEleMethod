#pragma once

#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"

namespace finelemethod::math
{
[[nodiscard]] DenseVector solve_gaussian_elimination(DenseMatrix matrix,
                                                     DenseVector right_hand_side);
} // namespace finelemethod::math
