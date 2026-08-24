#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::math::DenseMatrix;
using finelemethod::math::DenseVector;

TEST(DenseMatrix, ReportsDimensionsAndInitializesToZero)
{
    const DenseMatrix matrix(2, 3);

    EXPECT_EQ(matrix.rows(), 2);
    EXPECT_EQ(matrix.columns(), 3);
    EXPECT_EQ(matrix.size(), 6);

    for (DenseMatrix::size_type row = 0; row < matrix.rows(); ++row)
    {
        for (DenseMatrix::size_type column = 0; column < matrix.columns(); ++column)
        {
            EXPECT_DOUBLE_EQ(matrix(row, column), 0.0);
        }
    }
}

TEST(DenseMatrix, InitializesEveryEntryToRequestedValue)
{
    const DenseMatrix matrix(2, 2, 3.5);

    EXPECT_DOUBLE_EQ(matrix(0, 0), 3.5);
    EXPECT_DOUBLE_EQ(matrix(0, 1), 3.5);
    EXPECT_DOUBLE_EQ(matrix(1, 0), 3.5);
    EXPECT_DOUBLE_EQ(matrix(1, 1), 3.5);
}

TEST(DenseMatrix, ProvidesMutableElementAccess)
{
    DenseMatrix matrix(2, 2);

    matrix(0, 1) = 4.25;
    matrix(1, 0) = -2.0;

    const DenseMatrix &read_only_matrix = matrix;
    EXPECT_DOUBLE_EQ(read_only_matrix(0, 1), 4.25);
    EXPECT_DOUBLE_EQ(read_only_matrix(1, 0), -2.0);
}

TEST(DenseMatrix, RejectsZeroSizedDimensions)
{
    EXPECT_THROW(DenseMatrix(0, 2), std::invalid_argument);
    EXPECT_THROW(DenseMatrix(2, 0), std::invalid_argument);
    EXPECT_THROW(DenseMatrix(0, 0), std::invalid_argument);
}

TEST(DenseMatrix, RejectsDimensionsThatOverflowStorage)
{
    constexpr auto maximum_size = std::numeric_limits<DenseMatrix::size_type>::max();

    EXPECT_THROW(DenseMatrix(maximum_size, 2), std::length_error);
}

TEST(DenseMatrix, RejectsOutOfRangeIndices)
{
    DenseMatrix matrix(2, 3);

    EXPECT_THROW(matrix(2, 0), std::out_of_range);
    EXPECT_THROW(matrix(0, 3), std::out_of_range);
}

TEST(DenseMatrix, AddsMatricesWithMatchingDimensions)
{
    DenseMatrix left(2, 2);
    left(0, 0) = 1.0;
    left(0, 1) = 2.0;
    left(1, 0) = 3.0;
    left(1, 1) = 4.0;

    DenseMatrix right(2, 2);
    right(0, 0) = 5.0;
    right(0, 1) = 6.0;
    right(1, 0) = 7.0;
    right(1, 1) = 8.0;

    const DenseMatrix result = left + right;

    EXPECT_DOUBLE_EQ(result(0, 0), 6.0);
    EXPECT_DOUBLE_EQ(result(0, 1), 8.0);
    EXPECT_DOUBLE_EQ(result(1, 0), 10.0);
    EXPECT_DOUBLE_EQ(result(1, 1), 12.0);
    EXPECT_DOUBLE_EQ(left(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(right(0, 0), 5.0);
}

TEST(DenseMatrix, AddsIntoExistingMatrix)
{
    DenseMatrix left(1, 2, 1.5);
    const DenseMatrix right(1, 2, 2.0);

    DenseMatrix &returned_matrix = (left += right);

    EXPECT_EQ(&returned_matrix, &left);
    EXPECT_DOUBLE_EQ(left(0, 0), 3.5);
    EXPECT_DOUBLE_EQ(left(0, 1), 3.5);
}

TEST(DenseMatrix, RejectsAdditionWithMismatchedDimensions)
{
    const DenseMatrix two_by_two(2, 2);
    const DenseMatrix two_by_three(2, 3);
    const DenseMatrix three_by_two(3, 2);

    EXPECT_THROW(static_cast<void>(two_by_two + two_by_three), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(two_by_two + three_by_two), std::invalid_argument);
}

TEST(DenseMatrix, SubtractsMatricesWithMatchingDimensions)
{
    DenseMatrix left(2, 2);
    left(0, 0) = 8.0;
    left(0, 1) = 7.0;
    left(1, 0) = 6.0;
    left(1, 1) = 5.0;

    DenseMatrix right(2, 2);
    right(0, 0) = 1.0;
    right(0, 1) = 2.0;
    right(1, 0) = 3.0;
    right(1, 1) = 4.0;

    const DenseMatrix result = left - right;

    EXPECT_DOUBLE_EQ(result(0, 0), 7.0);
    EXPECT_DOUBLE_EQ(result(0, 1), 5.0);
    EXPECT_DOUBLE_EQ(result(1, 0), 3.0);
    EXPECT_DOUBLE_EQ(result(1, 1), 1.0);
    EXPECT_DOUBLE_EQ(left(0, 0), 8.0);
    EXPECT_DOUBLE_EQ(right(0, 0), 1.0);
}

TEST(DenseMatrix, SubtractsFromExistingMatrix)
{
    DenseMatrix left(1, 2, 5.0);
    const DenseMatrix right(1, 2, 1.5);

    DenseMatrix &returned_matrix = (left -= right);

    EXPECT_EQ(&returned_matrix, &left);
    EXPECT_DOUBLE_EQ(left(0, 0), 3.5);
    EXPECT_DOUBLE_EQ(left(0, 1), 3.5);
}

TEST(DenseMatrix, RejectsSubtractionWithMismatchedDimensions)
{
    const DenseMatrix two_by_two(2, 2);
    const DenseMatrix two_by_three(2, 3);
    const DenseMatrix three_by_two(3, 2);

    EXPECT_THROW(static_cast<void>(two_by_two - two_by_three), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(two_by_two - three_by_two), std::invalid_argument);
}

TEST(DenseMatrix, MultipliesByScalarWithoutChangingOriginal)
{
    DenseMatrix matrix(2, 2);
    matrix(0, 0) = 1.0;
    matrix(0, 1) = -2.0;
    matrix(1, 0) = 3.5;
    matrix(1, 1) = 4.0;

    const DenseMatrix result = matrix * -2.0;

    EXPECT_DOUBLE_EQ(result(0, 0), -2.0);
    EXPECT_DOUBLE_EQ(result(0, 1), 4.0);
    EXPECT_DOUBLE_EQ(result(1, 0), -7.0);
    EXPECT_DOUBLE_EQ(result(1, 1), -8.0);
    EXPECT_DOUBLE_EQ(matrix(0, 0), 1.0);
}

TEST(DenseMatrix, SupportsScalarOnLeft)
{
    const DenseMatrix matrix(1, 2, 2.5);

    const DenseMatrix result = 3.0 * matrix;

    EXPECT_DOUBLE_EQ(result(0, 0), 7.5);
    EXPECT_DOUBLE_EQ(result(0, 1), 7.5);
    EXPECT_DOUBLE_EQ(matrix(0, 0), 2.5);
}

TEST(DenseMatrix, MultipliesExistingMatrixByScalar)
{
    DenseMatrix matrix(1, 2, 3.0);

    DenseMatrix &returned_matrix = (matrix *= 0.0);

    EXPECT_EQ(&returned_matrix, &matrix);
    EXPECT_DOUBLE_EQ(matrix(0, 0), 0.0);
    EXPECT_DOUBLE_EQ(matrix(0, 1), 0.0);
}

TEST(DenseMatrix, MultipliesRectangularMatrices)
{
    DenseMatrix left(2, 3);
    left(0, 0) = 1.0;
    left(0, 1) = 2.0;
    left(0, 2) = 3.0;
    left(1, 0) = 4.0;
    left(1, 1) = 5.0;
    left(1, 2) = 6.0;

    DenseMatrix right(3, 2);
    right(0, 0) = 7.0;
    right(0, 1) = 8.0;
    right(1, 0) = 9.0;
    right(1, 1) = 10.0;
    right(2, 0) = 11.0;
    right(2, 1) = 12.0;

    const DenseMatrix result = left * right;

    EXPECT_EQ(result.rows(), 2);
    EXPECT_EQ(result.columns(), 2);
    EXPECT_DOUBLE_EQ(result(0, 0), 58.0);
    EXPECT_DOUBLE_EQ(result(0, 1), 64.0);
    EXPECT_DOUBLE_EQ(result(1, 0), 139.0);
    EXPECT_DOUBLE_EQ(result(1, 1), 154.0);
    EXPECT_DOUBLE_EQ(left(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(right(0, 0), 7.0);
}

TEST(DenseMatrix, PreservesMatrixWhenMultipliedByIdentity)
{
    DenseMatrix matrix(2, 2);
    matrix(0, 0) = 2.0;
    matrix(0, 1) = -1.0;
    matrix(1, 0) = 4.5;
    matrix(1, 1) = 3.0;

    DenseMatrix identity(2, 2);
    identity(0, 0) = 1.0;
    identity(1, 1) = 1.0;

    const DenseMatrix result = matrix * identity;

    EXPECT_DOUBLE_EQ(result(0, 0), 2.0);
    EXPECT_DOUBLE_EQ(result(0, 1), -1.0);
    EXPECT_DOUBLE_EQ(result(1, 0), 4.5);
    EXPECT_DOUBLE_EQ(result(1, 1), 3.0);
}

TEST(DenseMatrix, RejectsMultiplicationWithMismatchedInnerDimensions)
{
    const DenseMatrix two_by_three(2, 3);
    const DenseMatrix two_by_two(2, 2);

    EXPECT_THROW(static_cast<void>(two_by_three * two_by_two), std::invalid_argument);
}

TEST(DenseMatrix, MultipliesRectangularMatrixByVector)
{
    DenseMatrix matrix(2, 3);
    matrix(0, 0) = 1.0;
    matrix(0, 1) = 2.0;
    matrix(0, 2) = 3.0;
    matrix(1, 0) = 4.0;
    matrix(1, 1) = 5.0;
    matrix(1, 2) = 6.0;

    DenseVector vector(3);
    vector[0] = 2.0;
    vector[1] = -1.0;
    vector[2] = 0.5;

    const DenseVector result = matrix * vector;

    EXPECT_EQ(result.size(), 2);
    EXPECT_DOUBLE_EQ(result[0], 1.5);
    EXPECT_DOUBLE_EQ(result[1], 6.0);
}

TEST(DenseMatrix, IdentityMatrixPreservesVector)
{
    DenseMatrix identity(3, 3);
    identity(0, 0) = 1.0;
    identity(1, 1) = 1.0;
    identity(2, 2) = 1.0;

    DenseVector vector(3);
    vector[0] = -2.0;
    vector[1] = 4.5;
    vector[2] = 1.0;

    const DenseVector result = identity * vector;

    EXPECT_DOUBLE_EQ(result[0], -2.0);
    EXPECT_DOUBLE_EQ(result[1], 4.5);
    EXPECT_DOUBLE_EQ(result[2], 1.0);
}

TEST(DenseMatrix, RejectsMatrixVectorMultiplicationWithMismatchedDimensions)
{
    const DenseMatrix matrix(2, 3);
    const DenseVector vector(2);

    EXPECT_THROW(static_cast<void>(matrix * vector), std::invalid_argument);
}

TEST(DenseMatrix, TransposesRectangularMatrix)
{
    DenseMatrix matrix(2, 3);
    matrix(0, 0) = 1.0;
    matrix(0, 1) = 2.0;
    matrix(0, 2) = 3.0;
    matrix(1, 0) = 4.0;
    matrix(1, 1) = 5.0;
    matrix(1, 2) = 6.0;

    const DenseMatrix result = finelemethod::math::transpose(matrix);

    EXPECT_EQ(result.rows(), 3);
    EXPECT_EQ(result.columns(), 2);
    EXPECT_DOUBLE_EQ(result(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(result(0, 1), 4.0);
    EXPECT_DOUBLE_EQ(result(1, 0), 2.0);
    EXPECT_DOUBLE_EQ(result(1, 1), 5.0);
    EXPECT_DOUBLE_EQ(result(2, 0), 3.0);
    EXPECT_DOUBLE_EQ(result(2, 1), 6.0);
    EXPECT_DOUBLE_EQ(matrix(0, 1), 2.0);
}

TEST(DenseMatrix, DoubleTransposeRestoresOriginalMatrix)
{
    DenseMatrix matrix(2, 3);
    matrix(0, 0) = -1.0;
    matrix(0, 1) = 0.5;
    matrix(0, 2) = 8.0;
    matrix(1, 0) = 3.0;
    matrix(1, 1) = 2.0;
    matrix(1, 2) = -4.0;

    const DenseMatrix result = finelemethod::math::transpose(finelemethod::math::transpose(matrix));

    for (DenseMatrix::size_type row = 0; row < matrix.rows(); ++row)
    {
        for (DenseMatrix::size_type column = 0; column < matrix.columns(); ++column)
        {
            EXPECT_DOUBLE_EQ(result(row, column), matrix(row, column));
        }
    }
}
} // namespace
