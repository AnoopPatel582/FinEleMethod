#include "finelemethod/math/dense_matrix.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::math::DenseMatrix;

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
} // namespace
