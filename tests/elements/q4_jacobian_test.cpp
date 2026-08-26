#include "finelemethod/elements/q4_jacobian.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::elements::q4_jacobian;
using finelemethod::elements::Q4NodeCoordinates;

TEST(Q4Jacobian, ComputesAxisAlignedRectangleAtCenter)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 0.0}}, {{2.0, 1.0}}, {{0.0, 1.0}}}};

    const auto result = q4_jacobian(coordinates, 0.0, 0.0);

    EXPECT_DOUBLE_EQ(result.matrix(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(result.matrix(0, 1), 0.0);
    EXPECT_DOUBLE_EQ(result.matrix(1, 0), 0.0);
    EXPECT_DOUBLE_EQ(result.matrix(1, 1), 0.5);
    EXPECT_DOUBLE_EQ(result.determinant, 0.5);
    EXPECT_DOUBLE_EQ(result.inverse(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(result.inverse(0, 1), 0.0);
    EXPECT_DOUBLE_EQ(result.inverse(1, 0), 0.0);
    EXPECT_DOUBLE_EQ(result.inverse(1, 1), 2.0);
}

TEST(Q4Jacobian, ComputesSkewParallelogramAtArbitraryPoint)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{4.0, 0.0}}, {{5.0, 2.0}}, {{1.0, 2.0}}}};

    const auto result = q4_jacobian(coordinates, 0.3, -0.2);

    EXPECT_NEAR(result.matrix(0, 0), 2.0, 1.0e-15);
    EXPECT_NEAR(result.matrix(0, 1), 0.0, 1.0e-15);
    EXPECT_NEAR(result.matrix(1, 0), 0.5, 1.0e-15);
    EXPECT_NEAR(result.matrix(1, 1), 1.0, 1.0e-15);
    EXPECT_NEAR(result.determinant, 2.0, 1.0e-15);
    EXPECT_NEAR(result.inverse(0, 0), 0.5, 1.0e-15);
    EXPECT_NEAR(result.inverse(0, 1), 0.0, 1.0e-15);
    EXPECT_NEAR(result.inverse(1, 0), -0.25, 1.0e-15);
    EXPECT_NEAR(result.inverse(1, 1), 1.0, 1.0e-15);
}

TEST(Q4Jacobian, MatrixTimesInverseProducesIdentity)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{3.0, 0.5}}, {{2.5, 2.0}}, {{-0.5, 1.5}}}};
    const auto result = q4_jacobian(coordinates, -0.3, 0.4);

    const auto identity = result.matrix * result.inverse;

    EXPECT_NEAR(identity(0, 0), 1.0, 1.0e-14);
    EXPECT_NEAR(identity(0, 1), 0.0, 1.0e-14);
    EXPECT_NEAR(identity(1, 0), 0.0, 1.0e-14);
    EXPECT_NEAR(identity(1, 1), 1.0, 1.0e-14);
}

TEST(Q4Jacobian, RejectsClockwiseNodeOrder)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{0.0, 1.0}}, {{2.0, 1.0}}, {{2.0, 0.0}}}};

    EXPECT_THROW(static_cast<void>(q4_jacobian(coordinates, 0.0, 0.0)), std::invalid_argument);
}

TEST(Q4Jacobian, RejectsDegenerateGeometry)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{1.0, 0.0}}, {{2.0, 0.0}}, {{3.0, 0.0}}}};

    EXPECT_THROW(static_cast<void>(q4_jacobian(coordinates, 0.0, 0.0)), std::invalid_argument);
}

TEST(Q4Jacobian, RejectsNonfiniteNodalCoordinates)
{
    Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 0.0}}, {{2.0, 1.0}}, {{0.0, 1.0}}}};
    coordinates[2][0] = std::numeric_limits<double>::infinity();

    EXPECT_THROW(static_cast<void>(q4_jacobian(coordinates, 0.0, 0.0)), std::invalid_argument);
}
} // namespace
