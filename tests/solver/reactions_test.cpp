#include "finelemethod/solver/reactions.hpp"

#include "finelemethod/math/linear_solver.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::math::DenseMatrix;
using finelemethod::math::DenseVector;
using finelemethod::math::solve_gaussian_elimination;
using finelemethod::solver::apply_prescribed_displacement;
using finelemethod::solver::calculate_reaction_forces;

TEST(ReactionForces, CalculatesConstrainedReactionAndFreeResidual)
{
    DenseMatrix original_stiffness_matrix(2, 2);
    original_stiffness_matrix(0, 0) = 2.0;
    original_stiffness_matrix(0, 1) = -1.0;
    original_stiffness_matrix(1, 0) = -1.0;
    original_stiffness_matrix(1, 1) = 2.0;

    DenseVector original_load_vector(2);

    DenseMatrix constrained_stiffness_matrix = original_stiffness_matrix;
    DenseVector constrained_load_vector = original_load_vector;
    apply_prescribed_displacement(constrained_stiffness_matrix, constrained_load_vector, 0, 3.0);

    const DenseVector displacement =
        solve_gaussian_elimination(constrained_stiffness_matrix, constrained_load_vector);
    const DenseVector reactions =
        calculate_reaction_forces(original_stiffness_matrix, original_load_vector, displacement);

    EXPECT_DOUBLE_EQ(reactions[0], 4.5);
    EXPECT_DOUBLE_EQ(reactions[1], 0.0);
}

TEST(ReactionForces, IncludesOriginalAppliedLoads)
{
    DenseMatrix stiffness_matrix(2, 2);
    stiffness_matrix(0, 0) = 2.0;
    stiffness_matrix(0, 1) = -1.0;
    stiffness_matrix(1, 0) = -1.0;
    stiffness_matrix(1, 1) = 2.0;

    DenseVector load_vector(2);
    load_vector[1] = 1.0;

    DenseMatrix constrained_stiffness_matrix = stiffness_matrix;
    DenseVector constrained_load_vector = load_vector;
    apply_prescribed_displacement(constrained_stiffness_matrix, constrained_load_vector, 0, 0.0);

    const DenseVector displacement =
        solve_gaussian_elimination(constrained_stiffness_matrix, constrained_load_vector);
    const DenseVector reactions =
        calculate_reaction_forces(stiffness_matrix, load_vector, displacement);

    EXPECT_DOUBLE_EQ(reactions[0], -0.5);
    EXPECT_DOUBLE_EQ(reactions[1], 0.0);
}

TEST(ReactionForces, RejectsNonSquareStiffnessMatrix)
{
    const DenseMatrix stiffness_matrix(2, 3);
    const DenseVector load_vector(2);
    const DenseVector displacement_vector(3);

    EXPECT_THROW(static_cast<void>(
                     calculate_reaction_forces(stiffness_matrix, load_vector, displacement_vector)),
                 std::invalid_argument);
}

TEST(ReactionForces, RejectsMismatchedOriginalLoadVector)
{
    const DenseMatrix stiffness_matrix(2, 2);
    const DenseVector load_vector(3);
    const DenseVector displacement_vector(2);

    EXPECT_THROW(static_cast<void>(
                     calculate_reaction_forces(stiffness_matrix, load_vector, displacement_vector)),
                 std::invalid_argument);
}

TEST(ReactionForces, RejectsMismatchedDisplacementVector)
{
    const DenseMatrix stiffness_matrix(2, 2);
    const DenseVector load_vector(2);
    const DenseVector displacement_vector(3);

    EXPECT_THROW(static_cast<void>(
                     calculate_reaction_forces(stiffness_matrix, load_vector, displacement_vector)),
                 std::invalid_argument);
}
} // namespace
