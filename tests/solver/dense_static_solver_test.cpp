#include "finelemethod/solver/dense_static_solver.hpp"

#include <gtest/gtest.h>

#include <array>

namespace
{
using finelemethod::math::DenseMatrix;
using finelemethod::math::DenseVector;
using finelemethod::solver::PrescribedDisplacement;
using finelemethod::solver::solve_dense_static_system;

TEST(DenseStaticSolver, SolvesConstrainedSystemAndCalculatesReactions)
{
    DenseMatrix stiffness_matrix(3, 3);
    stiffness_matrix(0, 0) = 4.0;
    stiffness_matrix(0, 1) = -1.0;
    stiffness_matrix(1, 0) = -1.0;
    stiffness_matrix(1, 1) = 4.0;
    stiffness_matrix(1, 2) = -1.0;
    stiffness_matrix(2, 1) = -1.0;
    stiffness_matrix(2, 2) = 3.0;

    DenseVector load_vector(3);
    load_vector[1] = 5.0;

    const std::array constraints{PrescribedDisplacement{0, 2.0}, PrescribedDisplacement{2, -1.0}};

    const auto result = solve_dense_static_system(stiffness_matrix, load_vector, constraints);

    EXPECT_DOUBLE_EQ(result.displacements[0], 2.0);
    EXPECT_DOUBLE_EQ(result.displacements[1], 1.5);
    EXPECT_DOUBLE_EQ(result.displacements[2], -1.0);
    EXPECT_DOUBLE_EQ(result.reactions[0], 6.5);
    EXPECT_DOUBLE_EQ(result.reactions[1], 0.0);
    EXPECT_DOUBLE_EQ(result.reactions[2], -4.5);
}

TEST(DenseStaticSolver, PreservesOriginalSystem)
{
    DenseMatrix stiffness_matrix(2, 2);
    stiffness_matrix(0, 0) = 2.0;
    stiffness_matrix(0, 1) = -1.0;
    stiffness_matrix(1, 0) = -1.0;
    stiffness_matrix(1, 1) = 2.0;

    DenseVector load_vector(2);
    load_vector[1] = 1.0;

    const std::array constraints{PrescribedDisplacement{0, 0.0}};

    static_cast<void>(solve_dense_static_system(stiffness_matrix, load_vector, constraints));

    EXPECT_DOUBLE_EQ(stiffness_matrix(0, 0), 2.0);
    EXPECT_DOUBLE_EQ(stiffness_matrix(0, 1), -1.0);
    EXPECT_DOUBLE_EQ(stiffness_matrix(1, 0), -1.0);
    EXPECT_DOUBLE_EQ(stiffness_matrix(1, 1), 2.0);
    EXPECT_DOUBLE_EQ(load_vector[0], 0.0);
    EXPECT_DOUBLE_EQ(load_vector[1], 1.0);
}

TEST(DenseStaticSolver, SolvesUnconstrainedNonsingularSystem)
{
    DenseMatrix stiffness_matrix(2, 2);
    stiffness_matrix(0, 0) = 2.0;
    stiffness_matrix(1, 1) = 3.0;

    DenseVector load_vector(2);
    load_vector[0] = 4.0;
    load_vector[1] = 9.0;

    const std::array<PrescribedDisplacement, 0> constraints{};

    const auto result = solve_dense_static_system(stiffness_matrix, load_vector, constraints);

    EXPECT_DOUBLE_EQ(result.displacements[0], 2.0);
    EXPECT_DOUBLE_EQ(result.displacements[1], 3.0);
    EXPECT_DOUBLE_EQ(result.reactions[0], 0.0);
    EXPECT_DOUBLE_EQ(result.reactions[1], 0.0);
}
} // namespace
