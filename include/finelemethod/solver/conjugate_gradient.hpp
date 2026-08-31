#pragma once

#include "finelemethod/math/csr_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"

#include <cstddef>

namespace finelemethod::solver
{
struct ConjugateGradientOptions
{
    double relative_tolerance = 1.0e-10;
    double absolute_tolerance = 0.0;
    std::size_t maximum_iterations = 1000;
};

struct ConjugateGradientResult
{
    math::DenseVector solution;
    std::size_t iterations;
    double residual_norm;
    bool converged;
};

// Solves a symmetric positive-definite CSR system from a zero initial guess.
// A valid but unconverged result is returned when the iteration limit is met.
[[nodiscard]] ConjugateGradientResult solve_conjugate_gradient(
    const math::CsrMatrix &matrix, const math::DenseVector &right_hand_side,
    const ConjugateGradientOptions &options = {});
} // namespace finelemethod::solver
