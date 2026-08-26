#include "finelemethod/elements/q4_shape_functions.hpp"

#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::elements::q4_shape_function_natural_derivatives;
using finelemethod::elements::q4_shape_functions;

TEST(Q4ShapeFunctions, AreEqualAtElementCenter)
{
    const auto values = q4_shape_functions(0.0, 0.0);

    for (const double value : values)
    {
        EXPECT_DOUBLE_EQ(value, 0.25);
    }
}

TEST(Q4ShapeFunctions, SatisfyKroneckerDeltaPropertyAtNodes)
{
    constexpr std::array<std::array<double, 2>, 4> natural_nodes{
        {{{-1.0, -1.0}}, {{1.0, -1.0}}, {{1.0, 1.0}}, {{-1.0, 1.0}}}};

    for (std::size_t node = 0; node < natural_nodes.size(); ++node)
    {
        const auto values = q4_shape_functions(natural_nodes[node][0], natural_nodes[node][1]);

        for (std::size_t shape_function = 0; shape_function < values.size(); ++shape_function)
        {
            EXPECT_DOUBLE_EQ(values[shape_function], shape_function == node ? 1.0 : 0.0);
        }
    }
}

TEST(Q4ShapeFunctions, FormPartitionOfUnity)
{
    const auto values = q4_shape_functions(0.2, -0.4);

    double sum = 0.0;
    for (const double value : values)
    {
        sum += value;
    }

    EXPECT_NEAR(sum, 1.0, 1.0e-15);
}

TEST(Q4ShapeFunctions, ReproduceNaturalCoordinates)
{
    constexpr std::array<double, 4> nodal_xi{-1.0, 1.0, 1.0, -1.0};
    constexpr std::array<double, 4> nodal_eta{-1.0, -1.0, 1.0, 1.0};
    constexpr double xi = 0.2;
    constexpr double eta = -0.4;
    const auto values = q4_shape_functions(xi, eta);

    double interpolated_xi = 0.0;
    double interpolated_eta = 0.0;
    for (std::size_t node = 0; node < values.size(); ++node)
    {
        interpolated_xi += values[node] * nodal_xi[node];
        interpolated_eta += values[node] * nodal_eta[node];
    }

    EXPECT_NEAR(interpolated_xi, xi, 1.0e-15);
    EXPECT_NEAR(interpolated_eta, eta, 1.0e-15);
}

TEST(Q4ShapeFunctionNaturalDerivatives, HaveExpectedValuesAtElementCenter)
{
    const auto derivatives = q4_shape_function_natural_derivatives(0.0, 0.0);

    ASSERT_EQ(derivatives.rows(), 2);
    ASSERT_EQ(derivatives.columns(), 4);

    constexpr std::array<double, 4> expected_dxi{-0.25, 0.25, 0.25, -0.25};
    constexpr std::array<double, 4> expected_deta{-0.25, -0.25, 0.25, 0.25};
    for (std::size_t node = 0; node < expected_dxi.size(); ++node)
    {
        EXPECT_DOUBLE_EQ(derivatives(0, node), expected_dxi[node]);
        EXPECT_DOUBLE_EQ(derivatives(1, node), expected_deta[node]);
    }
}

TEST(Q4ShapeFunctionNaturalDerivatives, SumToZero)
{
    const auto derivatives = q4_shape_function_natural_derivatives(0.2, -0.4);

    double dxi_sum = 0.0;
    double deta_sum = 0.0;
    for (std::size_t node = 0; node < 4; ++node)
    {
        dxi_sum += derivatives(0, node);
        deta_sum += derivatives(1, node);
    }

    EXPECT_NEAR(dxi_sum, 0.0, 1.0e-15);
    EXPECT_NEAR(deta_sum, 0.0, 1.0e-15);
}

TEST(Q4ShapeFunctions, RejectNonfiniteNaturalCoordinates)
{
    const double infinity = std::numeric_limits<double>::infinity();
    const double not_a_number = std::numeric_limits<double>::quiet_NaN();

    EXPECT_THROW(static_cast<void>(q4_shape_functions(infinity, 0.0)), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(q4_shape_functions(0.0, not_a_number)), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(q4_shape_function_natural_derivatives(not_a_number, 0.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(q4_shape_function_natural_derivatives(0.0, infinity)),
                 std::invalid_argument);
}
} // namespace
