#include "finelemethod/elements/h8_jacobian.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

namespace
{
using finelemethod::elements::h8_jacobian;
using finelemethod::elements::H8NodeCoordinates;

H8NodeCoordinates axis_aligned_box()
{
    return {{{{0.0, 0.0, 0.0}},
             {{2.0, 0.0, 0.0}},
             {{2.0, 4.0, 0.0}},
             {{0.0, 4.0, 0.0}},
             {{0.0, 0.0, 6.0}},
             {{2.0, 0.0, 6.0}},
             {{2.0, 4.0, 6.0}},
             {{0.0, 4.0, 6.0}}}};
}

TEST(H8Jacobian, ComputesAxisAlignedBoxAtCenter)
{
    const auto result = h8_jacobian(axis_aligned_box(), 0.0, 0.0, 0.0);

    EXPECT_DOUBLE_EQ(result.matrix(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(result.matrix(1, 1), 2.0);
    EXPECT_DOUBLE_EQ(result.matrix(2, 2), 3.0);
    EXPECT_DOUBLE_EQ(result.matrix(0, 1), 0.0);
    EXPECT_DOUBLE_EQ(result.matrix(0, 2), 0.0);
    EXPECT_DOUBLE_EQ(result.matrix(1, 0), 0.0);
    EXPECT_DOUBLE_EQ(result.matrix(1, 2), 0.0);
    EXPECT_DOUBLE_EQ(result.matrix(2, 0), 0.0);
    EXPECT_DOUBLE_EQ(result.matrix(2, 1), 0.0);
    EXPECT_DOUBLE_EQ(result.determinant, 6.0);
    EXPECT_DOUBLE_EQ(result.inverse(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(result.inverse(1, 1), 0.5);
    EXPECT_DOUBLE_EQ(result.inverse(2, 2), 1.0 / 3.0);
}

TEST(H8Jacobian, ComputesSkewParallelepipedAtArbitraryPoint)
{
    const H8NodeCoordinates coordinates{{{{0.0, 0.0, 0.0}},
                                         {{2.0, 0.0, 0.0}},
                                         {{3.0, 4.0, 0.0}},
                                         {{1.0, 4.0, 0.0}},
                                         {{0.5, 1.0, 6.0}},
                                         {{2.5, 1.0, 6.0}},
                                         {{3.5, 5.0, 6.0}},
                                         {{1.5, 5.0, 6.0}}}};

    const auto result = h8_jacobian(coordinates, 0.3, -0.2, 0.4);

    EXPECT_NEAR(result.matrix(0, 0), 1.0, 1.0e-15);
    EXPECT_NEAR(result.matrix(1, 0), 0.5, 1.0e-15);
    EXPECT_NEAR(result.matrix(1, 1), 2.0, 1.0e-15);
    EXPECT_NEAR(result.matrix(2, 0), 0.25, 1.0e-15);
    EXPECT_NEAR(result.matrix(2, 1), 0.5, 1.0e-15);
    EXPECT_NEAR(result.matrix(2, 2), 3.0, 1.0e-15);
    EXPECT_NEAR(result.determinant, 6.0, 1.0e-14);
}

TEST(H8Jacobian, MatrixTimesInverseProducesIdentity)
{
    const auto result = h8_jacobian(axis_aligned_box(), -0.3, 0.4, -0.2);
    const auto identity = result.matrix * result.inverse;

    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            EXPECT_NEAR(identity(row, column), row == column ? 1.0 : 0.0, 1.0e-14);
        }
    }
}

TEST(H8Jacobian, RejectsInvertedAndDegenerateGeometry)
{
    H8NodeCoordinates inverted = axis_aligned_box();
    for (std::size_t node = 0; node < 4; ++node)
    {
        std::swap(inverted[node], inverted[node + 4]);
    }
    H8NodeCoordinates degenerate = axis_aligned_box();
    for (auto &coordinate : degenerate)
    {
        coordinate[2] = 0.0;
    }

    EXPECT_THROW(static_cast<void>(h8_jacobian(inverted, 0.0, 0.0, 0.0)), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(h8_jacobian(degenerate, 0.0, 0.0, 0.0)), std::invalid_argument);
}

TEST(H8Jacobian, RejectsNonfiniteNodalCoordinates)
{
    H8NodeCoordinates coordinates = axis_aligned_box();
    coordinates[6][2] = std::numeric_limits<double>::infinity();

    EXPECT_THROW(static_cast<void>(h8_jacobian(coordinates, 0.0, 0.0, 0.0)), std::invalid_argument);
}
} // namespace
