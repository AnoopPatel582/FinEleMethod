#pragma once

#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"

namespace finelemethod::solver
{
[[nodiscard]] math::DenseVector calculate_reaction_forces(
    const math::DenseMatrix &original_stiffness_matrix,
    const math::DenseVector &original_load_vector, const math::DenseVector &displacement_vector);
} // namespace finelemethod::solver
