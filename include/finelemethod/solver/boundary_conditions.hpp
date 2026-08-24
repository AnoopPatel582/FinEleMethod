#pragma once

#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"

#include <cstddef>

namespace finelemethod::solver
{
void apply_prescribed_displacement(math::DenseMatrix &stiffness_matrix,
                                   math::DenseVector &load_vector, std::size_t degree_of_freedom,
                                   double prescribed_value);
} // namespace finelemethod::solver
