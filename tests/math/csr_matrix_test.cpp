#include "finelemethod/math/coo_matrix.hpp"
#include "finelemethod/math/csr_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vector>

namespace
{
using finelemethod::math::convert_to_csr;
using finelemethod::math::CooMatrix;
using finelemethod::math::DenseVector;

TEST(CsrMatrix, ConvertsEmptyCoordinateMatrix)
{
    const CooMatrix coordinate_matrix(3, 4);
    const auto matrix = convert_to_csr(coordinate_matrix);

    EXPECT_EQ(matrix.rows(), 3U);
    EXPECT_EQ(matrix.columns(), 4U);
    EXPECT_EQ(matrix.nonzero_count(), 0U);
    EXPECT_TRUE(std::ranges::equal(matrix.row_offsets(), std::vector<std::size_t>{0, 0, 0, 0}));
    EXPECT_TRUE(matrix.column_indices().empty());
    EXPECT_TRUE(matrix.values().empty());
}

TEST(CsrMatrix, SortsEntriesIntoRowMajorOrder)
{
    CooMatrix coordinate_matrix(3, 4);
    coordinate_matrix.add(2, 1, 5.0);
    coordinate_matrix.add(0, 3, 2.0);
    coordinate_matrix.add(0, 1, -1.0);
    coordinate_matrix.add(2, 0, 4.0);

    const auto matrix = convert_to_csr(coordinate_matrix);

    EXPECT_TRUE(std::ranges::equal(matrix.row_offsets(), std::vector<std::size_t>{0, 2, 2, 4}));
    EXPECT_TRUE(std::ranges::equal(matrix.column_indices(), std::vector<std::size_t>{1, 3, 0, 1}));
    EXPECT_TRUE(std::ranges::equal(matrix.values(), std::vector<double>{-1.0, 2.0, 4.0, 5.0}));
}

TEST(CsrMatrix, SumsDuplicateCoordinates)
{
    CooMatrix coordinate_matrix(2, 2);
    coordinate_matrix.add(1, 0, 3.0);
    coordinate_matrix.add(0, 1, 4.0);
    coordinate_matrix.add(1, 0, -1.25);

    const auto matrix = convert_to_csr(coordinate_matrix);

    EXPECT_TRUE(std::ranges::equal(matrix.row_offsets(), std::vector<std::size_t>{0, 1, 2}));
    EXPECT_TRUE(std::ranges::equal(matrix.column_indices(), std::vector<std::size_t>{1, 0}));
    EXPECT_TRUE(std::ranges::equal(matrix.values(), std::vector<double>{4.0, 1.75}));
}

TEST(CsrMatrix, RemovesDuplicateContributionsThatCancelExactly)
{
    CooMatrix coordinate_matrix(2, 3);
    coordinate_matrix.add(0, 2, 7.0);
    coordinate_matrix.add(0, 2, -7.0);
    coordinate_matrix.add(1, 1, 3.0);

    const auto matrix = convert_to_csr(coordinate_matrix);

    EXPECT_TRUE(std::ranges::equal(matrix.row_offsets(), std::vector<std::size_t>{0, 0, 1}));
    EXPECT_TRUE(std::ranges::equal(matrix.column_indices(), std::vector<std::size_t>{1}));
    EXPECT_TRUE(std::ranges::equal(matrix.values(), std::vector<double>{3.0}));
}

TEST(CsrMatrix, DoesNotModifyCoordinateMatrix)
{
    CooMatrix coordinate_matrix(2, 2);
    coordinate_matrix.add(1, 1, 2.0);
    coordinate_matrix.add(0, 0, 1.0);

    static_cast<void>(convert_to_csr(coordinate_matrix));

    ASSERT_EQ(coordinate_matrix.entries().size(), 2U);
    EXPECT_EQ(coordinate_matrix.entries()[0].row, 1U);
    EXPECT_EQ(coordinate_matrix.entries()[1].row, 0U);
}

TEST(CsrMatrix, RejectsOverflowWhileSummingDuplicates)
{
    CooMatrix coordinate_matrix(1, 1);
    coordinate_matrix.add(0, 0, std::numeric_limits<double>::max());
    coordinate_matrix.add(0, 0, std::numeric_limits<double>::max());

    EXPECT_THROW(static_cast<void>(convert_to_csr(coordinate_matrix)), std::overflow_error);
}

TEST(CsrMatrix, MultipliesRectangularMatrixByDenseVector)
{
    CooMatrix coordinate_matrix(3, 4);
    coordinate_matrix.add(0, 0, 2.0);
    coordinate_matrix.add(0, 2, -1.0);
    coordinate_matrix.add(2, 1, 3.0);
    coordinate_matrix.add(2, 3, 4.0);
    const auto matrix = convert_to_csr(coordinate_matrix);
    DenseVector vector(4);
    vector[0] = 5.0;
    vector[1] = 2.0;
    vector[2] = -3.0;
    vector[3] = 0.5;

    const DenseVector result = matrix * vector;

    ASSERT_EQ(result.size(), 3U);
    EXPECT_DOUBLE_EQ(result[0], 13.0);
    EXPECT_DOUBLE_EQ(result[1], 0.0);
    EXPECT_DOUBLE_EQ(result[2], 8.0);
}

TEST(CsrMatrix, EmptyMatrixProducesZeroVector)
{
    const auto matrix = convert_to_csr(CooMatrix(2, 3));
    const DenseVector vector(3, 7.0);

    const DenseVector result = matrix * vector;

    ASSERT_EQ(result.size(), 2U);
    EXPECT_DOUBLE_EQ(result[0], 0.0);
    EXPECT_DOUBLE_EQ(result[1], 0.0);
}

TEST(CsrMatrix, RejectsVectorWithMismatchedSize)
{
    const auto matrix = convert_to_csr(CooMatrix(2, 3));
    const DenseVector vector(2);

    EXPECT_THROW(static_cast<void>(matrix * vector), std::invalid_argument);
}
} // namespace
