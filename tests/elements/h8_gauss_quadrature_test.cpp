#include "finelemethod/elements/h8_gauss_quadrature.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>

namespace
{
using finelemethod::elements::h8_gauss_quadrature_2x2x2;

TEST(H8GaussQuadrature, ProvidesEightSymmetricPointsWithUnitWeights)
{
    const auto points = h8_gauss_quadrature_2x2x2();
    const double coordinate = 1.0 / std::sqrt(3.0);
    const std::array<std::array<double, 3>, 8> expected{{
        {{-coordinate, -coordinate, -coordinate}},
        {{coordinate, -coordinate, -coordinate}},
        {{coordinate, coordinate, -coordinate}},
        {{-coordinate, coordinate, -coordinate}},
        {{-coordinate, -coordinate, coordinate}},
        {{coordinate, -coordinate, coordinate}},
        {{coordinate, coordinate, coordinate}},
        {{-coordinate, coordinate, coordinate}},
    }};

    ASSERT_EQ(points.size(), expected.size());
    for (std::size_t index = 0; index < points.size(); ++index)
    {
        EXPECT_DOUBLE_EQ(points[index].xi, expected[index][0]);
        EXPECT_DOUBLE_EQ(points[index].eta, expected[index][1]);
        EXPECT_DOUBLE_EQ(points[index].zeta, expected[index][2]);
        EXPECT_DOUBLE_EQ(points[index].weight, 1.0);
    }
}

TEST(H8GaussQuadrature, ExactlyIntegratesSupportedPolynomialTerms)
{
    const auto points = h8_gauss_quadrature_2x2x2();
    double constant_integral = 0.0;
    double xi_integral = 0.0;
    double zeta_squared_integral = 0.0;
    double mixed_odd_integral = 0.0;
    double squared_product_integral = 0.0;

    for (const auto &point : points)
    {
        constant_integral += point.weight;
        xi_integral += point.weight * point.xi;
        zeta_squared_integral += point.weight * point.zeta * point.zeta;
        mixed_odd_integral += point.weight * point.xi * point.eta * point.zeta;
        squared_product_integral +=
            point.weight * point.xi * point.xi * point.eta * point.eta * point.zeta * point.zeta;
    }

    EXPECT_NEAR(constant_integral, 8.0, 1.0e-15);
    EXPECT_NEAR(xi_integral, 0.0, 1.0e-15);
    EXPECT_NEAR(zeta_squared_integral, 8.0 / 3.0, 1.0e-15);
    EXPECT_NEAR(mixed_odd_integral, 0.0, 1.0e-15);
    EXPECT_NEAR(squared_product_integral, 8.0 / 27.0, 1.0e-15);
}
} // namespace
