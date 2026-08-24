#include "finelemethod/solver/boundary_conditions.hpp"

#include "finelemethod/math/linear_solver.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::math::DenseMatrix;
using finelemethod::math::DenseVector;
using finelemethod::math::solve_gaussian_elimination;
using finelemethod::solver::apply_prescribed_displacement;

TEST(PrescribedDisplacement, AppliesZeroValueByDirectElimination)
{
    DenseMatrix stiffness_matrix(3, 3);
    stiffness_matrix(0, 0) = 4.0;
    stiffness_matrix(0, 1) = -1.0;
    stiffness_matrix(0, 2) = 2.0;
    stiffness_matrix(1, 0) = -1.0;
    stiffness_matrix(1, 1) = 3.0;
    stiffness_matrix(1, 2) = -2.0;
    stiffness_matrix(2, 0) = 2.0;
    stiffness_matrix(2, 1) = -2.0;
    stiffness_matrix(2, 2) = 5.0;

    DenseVector load_vector(3);
    load_vector[0] = 6.0;
    load_vector[1] = 7.0;
    load_vector[2] = 8.0;

    apply_prescribed_displacement(stiffness_matrix, load_vector, 1, 0.0);

    EXPECT_DOUBLE_EQ(stiffness_matrix(0, 0), 4.0);
    EXPECT_DOUBLE_EQ(stiffness_matrix(0, 1), 0.0);
    EXPECT_DOUBLE_EQ(stiffness_matrix(0, 2), 2.0);
    EXPECT_DOUBLE_EQ(stiffness_matrix(1, 0), 0.0);
    EXPECT_DOUBLE_EQ(stiffness_matrix(1, 1), 1.0);
    EXPECT_DOUBLE_EQ(stiffness_matrix(1, 2), 0.0);
    EXPECT_DOUBLE_EQ(stiffness_matrix(2, 0), 2.0);
    EXPECT_DOUBLE_EQ(stiffness_matrix(2, 1), 0.0);
    EXPECT_DOUBLE_EQ(stiffness_matrix(2, 2), 5.0);
    EXPECT_DOUBLE_EQ(load_vector[0], 6.0);
    EXPECT_DOUBLE_EQ(load_vector[1], 0.0);
    EXPECT_DOUBLE_EQ(load_vector[2], 8.0);
}

TEST(PrescribedDisplacement, PreservesNonzeroValueInSolvedSystem)
{
    DenseMatrix stiffness_matrix(2, 2);
    stiffness_matrix(0, 0) = 2.0;
    stiffness_matrix(0, 1) = -1.0;
    stiffness_matrix(1, 0) = -1.0;
    stiffness_matrix(1, 1) = 2.0;

    DenseVector load_vector(2);

    apply_prescribed_displacement(stiffness_matrix, load_vector, 0, 3.0);
    const DenseVector displacement = solve_gaussian_elimination(stiffness_matrix, load_vector);

    EXPECT_DOUBLE_EQ(displacement[0], 3.0);
    EXPECT_DOUBLE_EQ(displacement[1], 1.5);
}

TEST(PrescribedDisplacement, RejectsNonSquareStiffnessMatrix)
{
    DenseMatrix stiffness_matrix(2, 3);
    DenseVector load_vector(2);

    EXPECT_THROW(apply_prescribed_displacement(stiffness_matrix, load_vector, 0, 0.0),
                 std::invalid_argument);
}

TEST(PrescribedDisplacement, RejectsMismatchedLoadVector)
{
    DenseMatrix stiffness_matrix(2, 2);
    DenseVector load_vector(3);

    EXPECT_THROW(apply_prescribed_displacement(stiffness_matrix, load_vector, 0, 0.0),
                 std::invalid_argument);
}

TEST(PrescribedDisplacement, RejectsOutOfRangeDegreeOfFreedom)
{
    DenseMatrix stiffness_matrix(2, 2);
    DenseVector load_vector(2);

    EXPECT_THROW(apply_prescribed_displacement(stiffness_matrix, load_vector, 2, 0.0),
                 std::out_of_range);
}
} // namespace
