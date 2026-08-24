#include "finelemethod/math/linear_solver.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::math::DenseMatrix;
using finelemethod::math::DenseVector;
using finelemethod::math::solve_gaussian_elimination;

TEST(GaussianElimination, SolvesThreeByThreeSystem)
{
    DenseMatrix matrix(3, 3);
    matrix(0, 0) = 2.0;
    matrix(0, 1) = 1.0;
    matrix(0, 2) = -1.0;
    matrix(1, 0) = -3.0;
    matrix(1, 1) = -1.0;
    matrix(1, 2) = 2.0;
    matrix(2, 0) = -2.0;
    matrix(2, 1) = 1.0;
    matrix(2, 2) = 2.0;

    DenseVector right_hand_side(3);
    right_hand_side[0] = 8.0;
    right_hand_side[1] = -11.0;
    right_hand_side[2] = -3.0;

    const DenseVector solution = solve_gaussian_elimination(matrix, right_hand_side);

    EXPECT_DOUBLE_EQ(solution[0], 2.0);
    EXPECT_DOUBLE_EQ(solution[1], 3.0);
    EXPECT_DOUBLE_EQ(solution[2], -1.0);
}

TEST(GaussianElimination, UsesPartialPivotingWhenLeadingEntryIsZero)
{
    DenseMatrix matrix(2, 2);
    matrix(0, 0) = 0.0;
    matrix(0, 1) = 2.0;
    matrix(1, 0) = 1.0;
    matrix(1, 1) = 1.0;

    DenseVector right_hand_side(2);
    right_hand_side[0] = 4.0;
    right_hand_side[1] = 3.0;

    const DenseVector solution = solve_gaussian_elimination(matrix, right_hand_side);

    EXPECT_DOUBLE_EQ(solution[0], 1.0);
    EXPECT_DOUBLE_EQ(solution[1], 2.0);
}

TEST(GaussianElimination, SolvesUniformlySmallScaledSystem)
{
    DenseMatrix matrix(2, 2);
    matrix(0, 0) = 1.0e-20;
    matrix(1, 1) = 2.0e-20;

    DenseVector right_hand_side(2);
    right_hand_side[0] = 3.0e-20;
    right_hand_side[1] = -8.0e-20;

    const DenseVector solution = solve_gaussian_elimination(matrix, right_hand_side);

    EXPECT_NEAR(solution[0], 3.0, 1.0e-12);
    EXPECT_NEAR(solution[1], -4.0, 1.0e-12);
}

TEST(GaussianElimination, RejectsNonSquareMatrix)
{
    const DenseMatrix matrix(2, 3);
    const DenseVector right_hand_side(2);

    EXPECT_THROW(static_cast<void>(solve_gaussian_elimination(matrix, right_hand_side)),
                 std::invalid_argument);
}

TEST(GaussianElimination, RejectsMismatchedRightHandSide)
{
    const DenseMatrix matrix(2, 2);
    const DenseVector right_hand_side(3);

    EXPECT_THROW(static_cast<void>(solve_gaussian_elimination(matrix, right_hand_side)),
                 std::invalid_argument);
}

TEST(GaussianElimination, RejectsSingularMatrix)
{
    DenseMatrix matrix(2, 2);
    matrix(0, 0) = 1.0;
    matrix(0, 1) = 2.0;
    matrix(1, 0) = 2.0;
    matrix(1, 1) = 4.0;

    DenseVector right_hand_side(2);
    right_hand_side[0] = 3.0;
    right_hand_side[1] = 6.0;

    EXPECT_THROW(static_cast<void>(solve_gaussian_elimination(matrix, right_hand_side)),
                 std::runtime_error);
}
} // namespace
