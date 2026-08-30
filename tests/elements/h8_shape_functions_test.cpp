#include "finelemethod/elements/h8_shape_functions.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::elements::h8_shape_function_natural_derivatives;
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

TEST(H8ShapeFunctionNaturalDerivatives, HaveExpectedValuesAtElementCenter)
{
    const auto derivatives = h8_shape_function_natural_derivatives(0.0, 0.0, 0.0);

    ASSERT_EQ(derivatives.rows(), 3U);
    ASSERT_EQ(derivatives.columns(), 8U);
    for (std::size_t node = 0; node < natural_nodes.size(); ++node)
    {
        EXPECT_DOUBLE_EQ(derivatives(0, node), 0.125 * natural_nodes[node][0]);
        EXPECT_DOUBLE_EQ(derivatives(1, node), 0.125 * natural_nodes[node][1]);
        EXPECT_DOUBLE_EQ(derivatives(2, node), 0.125 * natural_nodes[node][2]);
    }
}

TEST(H8ShapeFunctionNaturalDerivatives, SumToZero)
{
    const auto derivatives = h8_shape_function_natural_derivatives(0.2, -0.4, 0.6);

    for (std::size_t coordinate = 0; coordinate < derivatives.rows(); ++coordinate)
    {
        double sum = 0.0;
        for (std::size_t node = 0; node < derivatives.columns(); ++node)
        {
            sum += derivatives(coordinate, node);
        }
        EXPECT_NEAR(sum, 0.0, 1.0e-15);
    }
}

TEST(H8ShapeFunctionNaturalDerivatives, ReproduceNaturalCoordinateGradients)
{
    const auto derivatives = h8_shape_function_natural_derivatives(0.2, -0.4, 0.6);

    for (std::size_t derivative_coordinate = 0; derivative_coordinate < 3; ++derivative_coordinate)
    {
        for (std::size_t nodal_coordinate = 0; nodal_coordinate < 3; ++nodal_coordinate)
        {
            double gradient = 0.0;
            for (std::size_t node = 0; node < natural_nodes.size(); ++node)
            {
                gradient += derivatives(derivative_coordinate, node) *
                            natural_nodes[node][nodal_coordinate];
            }
            EXPECT_NEAR(gradient, derivative_coordinate == nodal_coordinate ? 1.0 : 0.0, 1.0e-15);
        }
    }
}

TEST(H8ShapeFunctionNaturalDerivatives, RejectNonfiniteNaturalCoordinates)
{
    const double infinity = std::numeric_limits<double>::infinity();
    const double not_a_number = std::numeric_limits<double>::quiet_NaN();

    EXPECT_THROW(static_cast<void>(h8_shape_function_natural_derivatives(infinity, 0.0, 0.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(h8_shape_function_natural_derivatives(0.0, not_a_number, 0.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(h8_shape_function_natural_derivatives(0.0, 0.0, infinity)),
                 std::invalid_argument);
}
} // namespace
