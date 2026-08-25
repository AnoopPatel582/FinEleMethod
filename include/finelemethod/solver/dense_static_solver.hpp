#pragma once

#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"

#include <span>

namespace finelemethod::solver
{
struct DenseStaticResult
{
    math::DenseVector displacements;
    math::DenseVector reactions;
};

[[nodiscard]] DenseStaticResult solve_dense_static_system(
    const math::DenseMatrix &stiffness_matrix, const math::DenseVector &load_vector,
    std::span<const PrescribedDisplacement> prescribed_displacements);
} // namespace finelemethod::solver
