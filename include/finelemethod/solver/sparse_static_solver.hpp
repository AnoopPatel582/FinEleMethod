#pragma once

#include "finelemethod/math/coo_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"
#include "finelemethod/solver/conjugate_gradient.hpp"

#include <cstddef>
#include <span>

namespace finelemethod::solver
{
struct SparseStaticSolution
{
    math::DenseVector displacements;
    math::DenseVector reactions;
    std::size_t iterations;
    double residual_norm;
};

// Applies sparse direct elimination, converts COO to CSR, solves with
// Conjugate Gradient, and calculates reactions from the original system.
[[nodiscard]] SparseStaticSolution solve_sparse_static_system(
    const math::CooMatrix &stiffness_matrix, const math::DenseVector &load_vector,
    std::span<const PrescribedDisplacement> prescribed_displacements,
    const ConjugateGradientOptions &options = {});
} // namespace finelemethod::solver
