#include "finelemethod/elements/q4_edge_pressure.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::elements::q4_uniform_edge_pressure_load;
using finelemethod::elements::Q4Edge;
using finelemethod::elements::Q4NodeCoordinates;

TEST(Q4EdgePressure, DistributesUniformPressureEquallyToEdgeNodes)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{4.0, 0.0}}, {{4.0, 2.0}}, {{0.0, 2.0}}}};

    const auto load = q4_uniform_edge_pressure_load(coordinates, Q4Edge::two, 10.0, 0.5);

    ASSERT_EQ(load.size(), 8U);
    EXPECT_DOUBLE_EQ(load[2], -5.0);
    EXPECT_DOUBLE_EQ(load[4], -5.0);
    for (const std::size_t dof : {0U, 1U, 3U, 5U, 6U, 7U})
    {
        EXPECT_DOUBLE_EQ(load[dof], 0.0);
    }
}

TEST(Q4EdgePressure, UsesInwardNormalsForEveryCounterclockwiseEdge)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{4.0, 0.0}}, {{4.0, 2.0}}, {{0.0, 2.0}}}};

    const auto bottom = q4_uniform_edge_pressure_load(coordinates, Q4Edge::one, 2.0, 1.0);
    const auto right = q4_uniform_edge_pressure_load(coordinates, Q4Edge::two, 2.0, 1.0);
    const auto top = q4_uniform_edge_pressure_load(coordinates, Q4Edge::three, 2.0, 1.0);
    const auto left = q4_uniform_edge_pressure_load(coordinates, Q4Edge::four, 2.0, 1.0);

    EXPECT_DOUBLE_EQ(bottom[1] + bottom[3], 8.0);
    EXPECT_DOUBLE_EQ(right[2] + right[4], -4.0);
    EXPECT_DOUBLE_EQ(top[5] + top[7], -8.0);
    EXPECT_DOUBLE_EQ(left[0] + left[6], 4.0);
}

TEST(Q4EdgePressure, ResolvesPressureNormalToSkewEdge)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 1.0}}, {{2.0, 3.0}}, {{0.0, 2.0}}}};

    const auto load = q4_uniform_edge_pressure_load(coordinates, Q4Edge::one, 6.0, 0.5);

    EXPECT_DOUBLE_EQ(load[0], -1.5);
    EXPECT_DOUBLE_EQ(load[1], 3.0);
    EXPECT_DOUBLE_EQ(load[2], -1.5);
    EXPECT_DOUBLE_EQ(load[3], 3.0);
}

TEST(Q4EdgePressure, RejectsInvalidInputs)
{
    const Q4NodeCoordinates valid{{{{0.0, 0.0}}, {{1.0, 0.0}}, {{1.0, 1.0}}, {{0.0, 1.0}}}};
    const Q4NodeCoordinates clockwise{{{{0.0, 0.0}}, {{0.0, 1.0}}, {{1.0, 1.0}}, {{1.0, 0.0}}}};
    const Q4NodeCoordinates zero_edge{{{{0.0, 0.0}}, {{0.0, 0.0}}, {{1.0, 1.0}}, {{0.0, 1.0}}}};
    const double infinity = std::numeric_limits<double>::infinity();

    EXPECT_THROW(
        static_cast<void>(q4_uniform_edge_pressure_load(valid, Q4Edge::one, infinity, 1.0)),
        std::invalid_argument);
    EXPECT_THROW(static_cast<void>(q4_uniform_edge_pressure_load(valid, Q4Edge::one, 1.0, 0.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(q4_uniform_edge_pressure_load(clockwise, Q4Edge::one, 1.0, 1.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(q4_uniform_edge_pressure_load(zero_edge, Q4Edge::one, 1.0, 1.0)),
                 std::invalid_argument);
    EXPECT_THROW(
        static_cast<void>(q4_uniform_edge_pressure_load(valid, static_cast<Q4Edge>(4), 1.0, 1.0)),
        std::invalid_argument);
}
} // namespace
