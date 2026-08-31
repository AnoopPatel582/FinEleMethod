#include "finelemethod/solver/sparse_static_solver.hpp"

#include "finelemethod/math/coo_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"

#include <gtest/gtest.h>

#include <array>
#include <stdexcept>

namespace
{
using finelemethod::math::CooMatrix;
using finelemethod::math::DenseVector;
using finelemethod::solver::ConjugateGradientOptions;
using finelemethod::solver::PrescribedDisplacement;
using finelemethod::solver::solve_sparse_static_system;

CooMatrix make_stiffness_matrix()
{
    CooMatrix matrix(3, 3);
    matrix.add(0, 0, 4.0);
    matrix.add(0, 1, -1.0);
    matrix.add(1, 0, -1.0);
    matrix.add(1, 1, 4.0);
    matrix.add(1, 2, -1.0);
    matrix.add(2, 1, -1.0);
    matrix.add(2, 2, 3.0);
    return matrix;
}

TEST(SparseStaticSolver, SolvesConstrainedSystemAndCalculatesReactions)
{
    const CooMatrix stiffness = make_stiffness_matrix();
    DenseVector loads(3);
    loads[1] = 5.0;
    const std::array constraints{PrescribedDisplacement{0, 2.0}, PrescribedDisplacement{2, -1.0}};

    const auto solution = solve_sparse_static_system(stiffness, loads, constraints);

    EXPECT_NEAR(solution.displacements[0], 2.0, 1.0e-12);
    EXPECT_NEAR(solution.displacements[1], 1.5, 1.0e-12);
    EXPECT_NEAR(solution.displacements[2], -1.0, 1.0e-12);
    EXPECT_NEAR(solution.reactions[0], 6.5, 1.0e-12);
    EXPECT_NEAR(solution.reactions[1], 0.0, 1.0e-12);
    EXPECT_NEAR(solution.reactions[2], -4.5, 1.0e-12);
    EXPECT_GT(solution.iterations, 0U);
    EXPECT_LT(solution.residual_norm, 1.0e-10);
}

TEST(SparseStaticSolver, PreservesOriginalSystem)
{
    const CooMatrix stiffness = make_stiffness_matrix();
    DenseVector loads(3);
    loads[1] = 5.0;
    const std::array constraints{PrescribedDisplacement{0, 0.0}};

    static_cast<void>(solve_sparse_static_system(stiffness, loads, constraints));

    EXPECT_EQ(stiffness.nonzero_count(), 7U);
    EXPECT_DOUBLE_EQ(stiffness.entries()[1].value, -1.0);
    EXPECT_DOUBLE_EQ(loads[0], 0.0);
    EXPECT_DOUBLE_EQ(loads[1], 5.0);
    EXPECT_DOUBLE_EQ(loads[2], 0.0);
}

TEST(SparseStaticSolver, SolvesUnconstrainedPositiveDefiniteSystem)
{
    CooMatrix stiffness(2, 2);
    stiffness.add(0, 0, 4.0);
    stiffness.add(0, 1, 1.0);
    stiffness.add(1, 0, 1.0);
    stiffness.add(1, 1, 3.0);
    DenseVector loads(2);
    loads[0] = 1.0;
    loads[1] = 2.0;
    const std::array<PrescribedDisplacement, 0> constraints{};

    const auto solution = solve_sparse_static_system(stiffness, loads, constraints);

    EXPECT_NEAR(solution.displacements[0], 1.0 / 11.0, 1.0e-14);
    EXPECT_NEAR(solution.displacements[1], 7.0 / 11.0, 1.0e-14);
    EXPECT_NEAR(solution.reactions[0], 0.0, 1.0e-14);
    EXPECT_NEAR(solution.reactions[1], 0.0, 1.0e-14);
}

TEST(SparseStaticSolver, ReportsIterationLimitAsNumericalFailure)
{
    CooMatrix stiffness(2, 2);
    stiffness.add(0, 0, 4.0);
    stiffness.add(0, 1, 1.0);
    stiffness.add(1, 0, 1.0);
    stiffness.add(1, 1, 3.0);
    const DenseVector loads(2, 1.0);
    const std::array<PrescribedDisplacement, 0> constraints{};
    ConjugateGradientOptions options;
    options.relative_tolerance = 1.0e-15;
    options.maximum_iterations = 1;

    EXPECT_THROW(
        static_cast<void>(solve_sparse_static_system(stiffness, loads, constraints, options)),
        std::runtime_error);
}
} // namespace
