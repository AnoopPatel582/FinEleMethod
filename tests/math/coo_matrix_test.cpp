#include "finelemethod/math/coo_matrix.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::math::CooMatrix;

TEST(CooMatrix, StartsEmptyAndReportsDimensions)
{
    const CooMatrix matrix(3, 4);

    EXPECT_EQ(matrix.rows(), 3U);
    EXPECT_EQ(matrix.columns(), 4U);
    EXPECT_EQ(matrix.nonzero_count(), 0U);
    EXPECT_TRUE(matrix.entries().empty());
}

TEST(CooMatrix, RejectsZeroSizedDimensions)
{
    EXPECT_THROW(CooMatrix(0, 2), std::invalid_argument);
    EXPECT_THROW(CooMatrix(2, 0), std::invalid_argument);
    EXPECT_THROW(CooMatrix(0, 0), std::invalid_argument);
}

TEST(CooMatrix, AppendsEntriesInAssemblyOrder)
{
    CooMatrix matrix(3, 4);

    matrix.add(2, 1, 4.5);
    matrix.add(0, 3, -2.0);

    ASSERT_EQ(matrix.nonzero_count(), 2U);
    EXPECT_EQ(matrix.entries()[0].row, 2U);
    EXPECT_EQ(matrix.entries()[0].column, 1U);
    EXPECT_DOUBLE_EQ(matrix.entries()[0].value, 4.5);
    EXPECT_EQ(matrix.entries()[1].row, 0U);
    EXPECT_EQ(matrix.entries()[1].column, 3U);
    EXPECT_DOUBLE_EQ(matrix.entries()[1].value, -2.0);
}

TEST(CooMatrix, RetainsDuplicateCoordinatesForLaterSummation)
{
    CooMatrix matrix(2, 2);

    matrix.add(1, 0, 3.0);
    matrix.add(1, 0, -1.25);

    ASSERT_EQ(matrix.nonzero_count(), 2U);
    EXPECT_EQ(matrix.entries()[0].row, matrix.entries()[1].row);
    EXPECT_EQ(matrix.entries()[0].column, matrix.entries()[1].column);
    EXPECT_DOUBLE_EQ(matrix.entries()[0].value, 3.0);
    EXPECT_DOUBLE_EQ(matrix.entries()[1].value, -1.25);
}

TEST(CooMatrix, IgnoresExplicitZeroContributions)
{
    CooMatrix matrix(2, 2);

    matrix.add(0, 1, 0.0);
    matrix.add(1, 0, -0.0);

    EXPECT_EQ(matrix.nonzero_count(), 0U);
}

TEST(CooMatrix, RejectsOutOfRangeCoordinates)
{
    CooMatrix matrix(2, 3);

    EXPECT_THROW(matrix.add(2, 0, 1.0), std::out_of_range);
    EXPECT_THROW(matrix.add(0, 3, 1.0), std::out_of_range);
    EXPECT_EQ(matrix.nonzero_count(), 0U);
}

TEST(CooMatrix, RejectsNonfiniteValues)
{
    CooMatrix matrix(2, 2);

    EXPECT_THROW(matrix.add(0, 0, std::numeric_limits<double>::infinity()), std::invalid_argument);
    EXPECT_THROW(matrix.add(0, 0, std::numeric_limits<double>::quiet_NaN()), std::invalid_argument);
    EXPECT_EQ(matrix.nonzero_count(), 0U);
}
} // namespace
