#include "finelemethod/elements/h8_shape_functions.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::elements::h8_shape_functions;

constexpr std::array<std::array<double, 3>, 8> natural_nodes{{
    {{-1.0, -1.0, -1.0}},
    {{1.0, -1.0, -1.0}},
    {{1.0, 1.0, -1.0}},
    {{-1.0, 1.0, -1.0}},
    {{-1.0, -1.0, 1.0}},
    {{1.0, -1.0, 1.0}},
    {{1.0, 1.0, 1.0}},
    {{-1.0, 1.0, 1.0}},
}};

TEST(H8ShapeFunctions, AreEqualAtElementCenter)
{
    const auto values = h8_shape_functions(0.0, 0.0, 0.0);

    for (const double value : values)
    {
        EXPECT_DOUBLE_EQ(value, 0.125);
    }
}

TEST(H8ShapeFunctions, SatisfyKroneckerDeltaPropertyAtNodes)
{
    for (std::size_t node = 0; node < natural_nodes.size(); ++node)
    {
        const auto values = h8_shape_functions(natural_nodes[node][0], natural_nodes[node][1],
                                               natural_nodes[node][2]);

        for (std::size_t shape_function = 0; shape_function < values.size(); ++shape_function)
        {
            EXPECT_DOUBLE_EQ(values[shape_function], shape_function == node ? 1.0 : 0.0);
        }
    }
}

TEST(H8ShapeFunctions, FormPartitionOfUnity)
{
    const auto values = h8_shape_functions(0.2, -0.4, 0.6);

    double sum = 0.0;
    for (const double value : values)
    {
        sum += value;
    }

    EXPECT_NEAR(sum, 1.0, 1.0e-15);
}

TEST(H8ShapeFunctions, ReproduceNaturalCoordinates)
{
    constexpr double xi = 0.2;
    constexpr double eta = -0.4;
    constexpr double zeta = 0.6;
    const auto values = h8_shape_functions(xi, eta, zeta);
    std::array<double, 3> interpolated{};

    for (std::size_t node = 0; node < values.size(); ++node)
    {
        for (std::size_t coordinate = 0; coordinate < interpolated.size(); ++coordinate)
        {
            interpolated[coordinate] += values[node] * natural_nodes[node][coordinate];
        }
    }

    EXPECT_NEAR(interpolated[0], xi, 1.0e-15);
    EXPECT_NEAR(interpolated[1], eta, 1.0e-15);
    EXPECT_NEAR(interpolated[2], zeta, 1.0e-15);
}

TEST(H8ShapeFunctions, RejectNonfiniteNaturalCoordinates)
{
    const double infinity = std::numeric_limits<double>::infinity();
    const double not_a_number = std::numeric_limits<double>::quiet_NaN();

    EXPECT_THROW(static_cast<void>(h8_shape_functions(infinity, 0.0, 0.0)), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(h8_shape_functions(0.0, not_a_number, 0.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(h8_shape_functions(0.0, 0.0, infinity)), std::invalid_argument);
}
} // namespace
