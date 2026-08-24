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

TEST(DenseVector, AddsVectorsWithMatchingSizes)
{
    DenseVector left(3);
    left[0] = 1.0;
    left[1] = -2.0;
    left[2] = 3.5;

    DenseVector right(3);
    right[0] = 4.0;
    right[1] = 2.5;
    right[2] = -1.5;

    const DenseVector result = left + right;

    EXPECT_DOUBLE_EQ(result[0], 5.0);
    EXPECT_DOUBLE_EQ(result[1], 0.5);
    EXPECT_DOUBLE_EQ(result[2], 2.0);
    EXPECT_DOUBLE_EQ(left[0], 1.0);
}

TEST(DenseVector, AddsIntoExistingVector)
{
    DenseVector left(2, 1.5);
    const DenseVector right(2, 2.0);

    DenseVector &result = (left += right);

    EXPECT_EQ(&result, &left);
    EXPECT_DOUBLE_EQ(left[0], 3.5);
    EXPECT_DOUBLE_EQ(left[1], 3.5);
}

TEST(DenseVector, RejectsAdditionWithMismatchedSizes)
{
    const DenseVector left(2);
    const DenseVector right(3);

    EXPECT_THROW(static_cast<void>(left + right), std::invalid_argument);
}

TEST(DenseVector, SubtractsVectorsWithMatchingSizes)
{
    DenseVector left(3);
    left[0] = 5.0;
    left[1] = -2.0;
    left[2] = 3.5;

    DenseVector right(3);
    right[0] = 1.0;
    right[1] = 2.5;
    right[2] = -1.5;

    const DenseVector result = left - right;

    EXPECT_DOUBLE_EQ(result[0], 4.0);
    EXPECT_DOUBLE_EQ(result[1], -4.5);
    EXPECT_DOUBLE_EQ(result[2], 5.0);
    EXPECT_DOUBLE_EQ(left[0], 5.0);
}

TEST(DenseVector, SubtractsFromExistingVector)
{
    DenseVector left(2, 4.0);
    const DenseVector right(2, 1.5);

    DenseVector &result = (left -= right);

    EXPECT_EQ(&result, &left);
    EXPECT_DOUBLE_EQ(left[0], 2.5);
    EXPECT_DOUBLE_EQ(left[1], 2.5);
}

TEST(DenseVector, RejectsSubtractionWithMismatchedSizes)
{
    const DenseVector left(2);
    const DenseVector right(3);

    EXPECT_THROW(static_cast<void>(left - right), std::invalid_argument);
}

TEST(DenseVector, MultipliesByScalarWithoutChangingOriginal)
{
    DenseVector vector(3);
    vector[0] = 1.0;
    vector[1] = -2.0;
    vector[2] = 0.5;

    const DenseVector result = vector * 3.0;

    EXPECT_DOUBLE_EQ(result[0], 3.0);
    EXPECT_DOUBLE_EQ(result[1], -6.0);
    EXPECT_DOUBLE_EQ(result[2], 1.5);
    EXPECT_DOUBLE_EQ(vector[0], 1.0);
}

TEST(DenseVector, SupportsScalarOnLeft)
{
    DenseVector vector(2);
    vector[0] = 2.0;
    vector[1] = -4.0;

    const DenseVector result = 0.5 * vector;

    EXPECT_DOUBLE_EQ(result[0], 1.0);
    EXPECT_DOUBLE_EQ(result[1], -2.0);
}

TEST(DenseVector, MultipliesExistingVectorByScalar)
{
    DenseVector vector(2, 2.0);

    DenseVector &result = (vector *= -1.5);

    EXPECT_EQ(&result, &vector);
    EXPECT_DOUBLE_EQ(vector[0], -3.0);
    EXPECT_DOUBLE_EQ(vector[1], -3.0);
}
} // namespace
