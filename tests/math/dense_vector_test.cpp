#include "finelemethod/math/dense_vector.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::math::DenseVector;

TEST(DenseVector, ReportsSizeAndInitializesToZero)
{
    const DenseVector vector(3);

    EXPECT_EQ(vector.size(), 3);
    EXPECT_DOUBLE_EQ(vector[0], 0.0);
    EXPECT_DOUBLE_EQ(vector[1], 0.0);
    EXPECT_DOUBLE_EQ(vector[2], 0.0);
}

TEST(DenseVector, InitializesEveryEntryToRequestedValue)
{
    const DenseVector vector(3, -2.5);

    EXPECT_DOUBLE_EQ(vector[0], -2.5);
    EXPECT_DOUBLE_EQ(vector[1], -2.5);
    EXPECT_DOUBLE_EQ(vector[2], -2.5);
}

TEST(DenseVector, ProvidesMutableElementAccess)
{
    DenseVector vector(2);

    vector[0] = 4.25;
    vector[1] = -3.0;

    const DenseVector &read_only_vector = vector;
    EXPECT_DOUBLE_EQ(read_only_vector[0], 4.25);
    EXPECT_DOUBLE_EQ(read_only_vector[1], -3.0);
}

TEST(DenseVector, RejectsZeroSize)
{
    EXPECT_THROW(DenseVector(0), std::invalid_argument);
}

TEST(DenseVector, RejectsOutOfRangeIndex)
{
    DenseVector vector(2);

    EXPECT_THROW(vector[2], std::out_of_range);
}
} // namespace
