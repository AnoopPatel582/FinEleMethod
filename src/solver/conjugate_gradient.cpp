#include "finelemethod/solver/conjugate_gradient.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace finelemethod::solver
{
ConjugateGradientResult solve_conjugate_gradient(const math::CsrMatrix &matrix,
                                                 const math::DenseVector &right_hand_side,
                                                 const ConjugateGradientOptions &options)
{
    if (matrix.rows() != matrix.columns())
    {
        throw std::invalid_argument("Conjugate Gradient requires a square matrix.");
    }
    if (right_hand_side.size() != matrix.rows())
    {
        throw std::invalid_argument(
            "Conjugate Gradient right-hand-side size must match the matrix.");
    }
    if (!std::isfinite(options.relative_tolerance) || options.relative_tolerance < 0.0 ||
        !std::isfinite(options.absolute_tolerance) || options.absolute_tolerance < 0.0 ||
        (options.relative_tolerance == 0.0 && options.absolute_tolerance == 0.0))
    {
        throw std::invalid_argument(
            "Conjugate Gradient tolerances must be finite, nonnegative, and not both zero.");
    }
    if (options.maximum_iterations == 0)
    {
        throw std::invalid_argument(
            "Conjugate Gradient maximum iterations must be greater than zero.");
    }
    for (std::size_t index = 0; index < right_hand_side.size(); ++index)
    {
        if (!std::isfinite(right_hand_side[index]))
        {
            throw std::invalid_argument("Conjugate Gradient right-hand side must be finite.");
        }
    }

    math::DenseVector solution(matrix.columns());
    math::DenseVector residual = right_hand_side;
    math::DenseVector direction = residual;
    double residual_squared = math::dot(residual, residual);
    const double right_hand_side_norm = std::sqrt(residual_squared);
    const double convergence_tolerance =
        std::max(options.absolute_tolerance, options.relative_tolerance * right_hand_side_norm);

    if (right_hand_side_norm <= convergence_tolerance)
    {
        return ConjugateGradientResult{std::move(solution), 0, right_hand_side_norm, true};
    }

    for (std::size_t iteration = 1; iteration <= options.maximum_iterations; ++iteration)
    {
        const math::DenseVector matrix_direction = matrix * direction;
        const double curvature = math::dot(direction, matrix_direction);
        if (!std::isfinite(curvature) || curvature <= 0.0)
        {
            throw std::runtime_error(
                "Conjugate Gradient encountered a nonpositive or nonfinite curvature.");
        }

        const double step = residual_squared / curvature;
        if (!std::isfinite(step))
        {
            throw std::runtime_error("Conjugate Gradient produced a nonfinite step.");
        }
        solution += step * direction;
        residual -= step * matrix_direction;

        const double next_residual_squared = math::dot(residual, residual);
        if (!std::isfinite(next_residual_squared))
        {
            throw std::runtime_error("Conjugate Gradient produced a nonfinite residual.");
        }
        const double residual_norm = std::sqrt(next_residual_squared);
        if (residual_norm <= convergence_tolerance)
        {
            return ConjugateGradientResult{std::move(solution), iteration, residual_norm, true};
        }

        const double direction_scale = next_residual_squared / residual_squared;
        direction = residual + direction_scale * direction;
        residual_squared = next_residual_squared;
    }

    return ConjugateGradientResult{std::move(solution), options.maximum_iterations,
                                   std::sqrt(residual_squared), false};
}
} // namespace finelemethod::solver
