#include "finelemethod/elements/q4_gauss_quadrature.hpp"

#include <gtest/gtest.h>

#include <cmath>

namespace
{
using finelemethod::elements::q4_gauss_quadrature_2x2;

TEST(Q4GaussQuadrature, ProvidesFourSymmetricPointsWithUnitWeights)
{
    const auto points = q4_gauss_quadrature_2x2();
    const double coordinate = 1.0 / std::sqrt(3.0);

    ASSERT_EQ(points.size(), 4);
    EXPECT_DOUBLE_EQ(points[0].xi, -coordinate);
    EXPECT_DOUBLE_EQ(points[0].eta, -coordinate);
    EXPECT_DOUBLE_EQ(points[1].xi, coordinate);
    EXPECT_DOUBLE_EQ(points[1].eta, -coordinate);
    EXPECT_DOUBLE_EQ(points[2].xi, coordinate);
    EXPECT_DOUBLE_EQ(points[2].eta, coordinate);
    EXPECT_DOUBLE_EQ(points[3].xi, -coordinate);
    EXPECT_DOUBLE_EQ(points[3].eta, coordinate);

    for (const auto &point : points)
    {
        EXPECT_DOUBLE_EQ(point.weight, 1.0);
    }
}

TEST(Q4GaussQuadrature, ExactlyIntegratesSupportedPolynomialTerms)
{
    const auto points = q4_gauss_quadrature_2x2();
    double constant_integral = 0.0;
    double xi_integral = 0.0;
    double eta_integral = 0.0;
    double xi_squared_integral = 0.0;
    double eta_squared_integral = 0.0;
    double xi_eta_integral = 0.0;

    for (const auto &point : points)
    {
        constant_integral += point.weight;
        xi_integral += point.weight * point.xi;
        eta_integral += point.weight * point.eta;
        xi_squared_integral += point.weight * point.xi * point.xi;
        eta_squared_integral += point.weight * point.eta * point.eta;
        xi_eta_integral += point.weight * point.xi * point.eta;
    }

    EXPECT_NEAR(constant_integral, 4.0, 1.0e-15);
    EXPECT_NEAR(xi_integral, 0.0, 1.0e-15);
    EXPECT_NEAR(eta_integral, 0.0, 1.0e-15);
    EXPECT_NEAR(xi_squared_integral, 4.0 / 3.0, 1.0e-15);
    EXPECT_NEAR(eta_squared_integral, 4.0 / 3.0, 1.0e-15);
    EXPECT_NEAR(xi_eta_integral, 0.0, 1.0e-15);
}
} // namespace
