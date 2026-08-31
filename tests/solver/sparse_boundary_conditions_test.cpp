#include "finelemethod/solver/boundary_conditions.hpp"

#include "finelemethod/math/coo_matrix.hpp"
#include "finelemethod/math/csr_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/solver/conjugate_gradient.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::math::convert_to_csr;
using finelemethod::math::CooMatrix;
using finelemethod::math::DenseVector;
using finelemethod::solver::apply_prescribed_displacements;
using finelemethod::solver::PrescribedDisplacement;
using finelemethod::solver::solve_conjugate_gradient;

double csr_value(const finelemethod::math::CsrMatrix &matrix, const std::size_t row,
                 const std::size_t column)
{
    for (std::size_t index = matrix.row_offsets()[row]; index < matrix.row_offsets()[row + 1];
         ++index)
    {
        if (matrix.column_indices()[index] == column)
        {
            return matrix.values()[index];
        }
    }
    return 0.0;
}

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

TEST(SparsePrescribedDisplacement, AppliesMultipleValuesAndPreservesSymmetry)
{
    const CooMatrix stiffness = make_stiffness_matrix();
    DenseVector loads(3);
    loads[1] = 5.0;
    const std::array constraints{PrescribedDisplacement{0, 2.0}, PrescribedDisplacement{2, -1.0}};

    const auto constrained = apply_prescribed_displacements(stiffness, loads, constraints);
    const auto matrix = convert_to_csr(constrained.stiffness_matrix);

    EXPECT_DOUBLE_EQ(csr_value(matrix, 0, 0), 1.0);
    EXPECT_DOUBLE_EQ(csr_value(matrix, 1, 1), 4.0);
    EXPECT_DOUBLE_EQ(csr_value(matrix, 2, 2), 1.0);
    EXPECT_DOUBLE_EQ(csr_value(matrix, 0, 1), 0.0);
    EXPECT_DOUBLE_EQ(csr_value(matrix, 1, 0), 0.0);
    EXPECT_DOUBLE_EQ(csr_value(matrix, 1, 2), 0.0);
    EXPECT_DOUBLE_EQ(csr_value(matrix, 2, 1), 0.0);
    EXPECT_DOUBLE_EQ(constrained.load_vector[0], 2.0);
    EXPECT_DOUBLE_EQ(constrained.load_vector[1], 6.0);
    EXPECT_DOUBLE_EQ(constrained.load_vector[2], -1.0);
}

TEST(SparsePrescribedDisplacement, ProducesSystemSolvedByConjugateGradient)
{
    const CooMatrix stiffness = make_stiffness_matrix();
    DenseVector loads(3);
    loads[1] = 5.0;
    const std::array constraints{PrescribedDisplacement{0, 2.0}, PrescribedDisplacement{2, -1.0}};
    const auto constrained = apply_prescribed_displacements(stiffness, loads, constraints);

    const auto result = solve_conjugate_gradient(convert_to_csr(constrained.stiffness_matrix),
                                                 constrained.load_vector);

    ASSERT_TRUE(result.converged);
    EXPECT_NEAR(result.solution[0], 2.0, 1.0e-12);
    EXPECT_NEAR(result.solution[1], 1.5, 1.0e-12);
    EXPECT_NEAR(result.solution[2], -1.0, 1.0e-12);
}

TEST(SparsePrescribedDisplacement, CombinesDuplicateColumnContributionsInLoadAdjustment)
{
    CooMatrix stiffness(2, 2);
    stiffness.add(0, 0, 2.0);
    stiffness.add(0, 1, -0.25);
    stiffness.add(0, 1, -0.75);
    stiffness.add(1, 0, -1.0);
    stiffness.add(1, 1, 2.0);
    const std::array constraints{PrescribedDisplacement{1, 3.0}};

    const auto constrained = apply_prescribed_displacements(stiffness, DenseVector(2), constraints);

    EXPECT_DOUBLE_EQ(constrained.load_vector[0], 3.0);
    EXPECT_DOUBLE_EQ(constrained.load_vector[1], 3.0);
}

TEST(SparsePrescribedDisplacement, EmptyConstraintsPreserveSystem)
{
    const CooMatrix stiffness = make_stiffness_matrix();
    const DenseVector loads(3, 2.0);
    const std::array<PrescribedDisplacement, 0> constraints{};

    const auto constrained = apply_prescribed_displacements(stiffness, loads, constraints);

    EXPECT_EQ(constrained.stiffness_matrix.nonzero_count(), stiffness.nonzero_count());
    EXPECT_DOUBLE_EQ(constrained.load_vector[0], 2.0);
    EXPECT_DOUBLE_EQ(constrained.load_vector[1], 2.0);
    EXPECT_DOUBLE_EQ(constrained.load_vector[2], 2.0);
}

TEST(SparsePrescribedDisplacement, RejectsInvalidInput)
{
    const CooMatrix rectangular(2, 3);
    const CooMatrix square(2, 2);
    const std::array out_of_range{PrescribedDisplacement{2, 0.0}};
    const std::array duplicate{PrescribedDisplacement{0, 0.0}, PrescribedDisplacement{0, 1.0}};
    const std::array nonfinite{PrescribedDisplacement{0, std::numeric_limits<double>::infinity()}};

    EXPECT_THROW(static_cast<void>(apply_prescribed_displacements(
                     rectangular, DenseVector(2), std::span<const PrescribedDisplacement>{})),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(apply_prescribed_displacements(
                     square, DenseVector(3), std::span<const PrescribedDisplacement>{})),
                 std::invalid_argument);
    EXPECT_THROW(
        static_cast<void>(apply_prescribed_displacements(square, DenseVector(2), out_of_range)),
        std::out_of_range);
    EXPECT_THROW(
        static_cast<void>(apply_prescribed_displacements(square, DenseVector(2), duplicate)),
        std::invalid_argument);
    EXPECT_THROW(
        static_cast<void>(apply_prescribed_displacements(square, DenseVector(2), nonfinite)),
        std::invalid_argument);
}
} // namespace
