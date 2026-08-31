#include "finelemethod/math/coo_matrix.hpp"
#include "finelemethod/math/csr_matrix.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vector>

namespace
{
using finelemethod::math::convert_to_csr;
using finelemethod::math::CooMatrix;

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
} // namespace
