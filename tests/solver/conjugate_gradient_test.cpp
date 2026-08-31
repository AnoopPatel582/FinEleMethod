#include "finelemethod/solver/conjugate_gradient.hpp"

#include "finelemethod/math/coo_matrix.hpp"
#include "finelemethod/math/csr_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::math::convert_to_csr;
using finelemethod::math::CooMatrix;
using finelemethod::math::DenseVector;
using finelemethod::solver::ConjugateGradientOptions;
using finelemethod::solver::solve_conjugate_gradient;

TEST(ConjugateGradient, SolvesSymmetricPositiveDefiniteSystem)
{
    CooMatrix coordinate_matrix(2, 2);
    coordinate_matrix.add(0, 0, 4.0);
    coordinate_matrix.add(0, 1, 1.0);
    coordinate_matrix.add(1, 0, 1.0);
    coordinate_matrix.add(1, 1, 3.0);
    const auto matrix = convert_to_csr(coordinate_matrix);
    DenseVector right_hand_side(2);
    right_hand_side[0] = 1.0;
    right_hand_side[1] = 2.0;

    const auto result = solve_conjugate_gradient(matrix, right_hand_side);

    EXPECT_TRUE(result.converged);
    EXPECT_EQ(result.iterations, 2U);
    EXPECT_NEAR(result.solution[0], 1.0 / 11.0, 1.0e-14);
    EXPECT_NEAR(result.solution[1], 7.0 / 11.0, 1.0e-14);
    EXPECT_LT(result.residual_norm, 1.0e-14);
}

TEST(ConjugateGradient, ZeroRightHandSideConvergesWithoutIteration)
{
    CooMatrix coordinate_matrix(2, 2);
    coordinate_matrix.add(0, 0, 2.0);
    coordinate_matrix.add(1, 1, 3.0);
    const auto matrix = convert_to_csr(coordinate_matrix);

    const auto result = solve_conjugate_gradient(matrix, DenseVector(2));

    EXPECT_TRUE(result.converged);
    EXPECT_EQ(result.iterations, 0U);
    EXPECT_DOUBLE_EQ(result.residual_norm, 0.0);
    EXPECT_DOUBLE_EQ(result.solution[0], 0.0);
    EXPECT_DOUBLE_EQ(result.solution[1], 0.0);
}

TEST(ConjugateGradient, ReportsIterationLimitWithoutThrowing)
{
    CooMatrix coordinate_matrix(2, 2);
    coordinate_matrix.add(0, 0, 4.0);
    coordinate_matrix.add(0, 1, 1.0);
    coordinate_matrix.add(1, 0, 1.0);
    coordinate_matrix.add(1, 1, 3.0);
    const auto matrix = convert_to_csr(coordinate_matrix);
    const DenseVector right_hand_side(2, 1.0);
    ConjugateGradientOptions options;
    options.relative_tolerance = 1.0e-15;
    options.maximum_iterations = 1;

    const auto result = solve_conjugate_gradient(matrix, right_hand_side, options);

    EXPECT_FALSE(result.converged);
    EXPECT_EQ(result.iterations, 1U);
    EXPECT_GT(result.residual_norm, 0.0);
}

TEST(ConjugateGradient, RejectsInvalidDimensions)
{
    const auto rectangular = convert_to_csr(CooMatrix(2, 3));
    const auto square = convert_to_csr(CooMatrix(2, 2));

    EXPECT_THROW(static_cast<void>(solve_conjugate_gradient(rectangular, DenseVector(2))),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(solve_conjugate_gradient(square, DenseVector(3))),
                 std::invalid_argument);
}

TEST(ConjugateGradient, RejectsInvalidOptions)
{
    CooMatrix coordinate_matrix(1, 1);
    coordinate_matrix.add(0, 0, 1.0);
    const auto matrix = convert_to_csr(coordinate_matrix);
    const DenseVector right_hand_side(1, 1.0);

    ConjugateGradientOptions options;
    options.relative_tolerance = -1.0;
    EXPECT_THROW(static_cast<void>(solve_conjugate_gradient(matrix, right_hand_side, options)),
                 std::invalid_argument);

    options.relative_tolerance = 0.0;
    options.absolute_tolerance = 0.0;
    EXPECT_THROW(static_cast<void>(solve_conjugate_gradient(matrix, right_hand_side, options)),
                 std::invalid_argument);

    options.relative_tolerance = 1.0e-10;
    options.maximum_iterations = 0;
    EXPECT_THROW(static_cast<void>(solve_conjugate_gradient(matrix, right_hand_side, options)),
                 std::invalid_argument);
}

TEST(ConjugateGradient, RejectsNonfiniteRightHandSide)
{
    CooMatrix coordinate_matrix(1, 1);
    coordinate_matrix.add(0, 0, 1.0);
    const auto matrix = convert_to_csr(coordinate_matrix);
    const DenseVector right_hand_side(1, std::numeric_limits<double>::quiet_NaN());

    EXPECT_THROW(static_cast<void>(solve_conjugate_gradient(matrix, right_hand_side)),
                 std::invalid_argument);
}

TEST(ConjugateGradient, RejectsNonpositiveCurvature)
{
    CooMatrix coordinate_matrix(1, 1);
    coordinate_matrix.add(0, 0, -1.0);
    const auto matrix = convert_to_csr(coordinate_matrix);

    EXPECT_THROW(static_cast<void>(solve_conjugate_gradient(matrix, DenseVector(1, 1.0))),
                 std::runtime_error);
}
} // namespace
