#include "finelemethod/elements/q4_physical_derivatives.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::elements::q4_shape_function_physical_derivatives;
using finelemethod::elements::Q4NodeCoordinates;

TEST(Q4PhysicalDerivatives, ComputesAxisAlignedRectangleAtCenter)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 0.0}}, {{2.0, 1.0}}, {{0.0, 1.0}}}};

    const auto result = q4_shape_function_physical_derivatives(coordinates, 0.0, 0.0);

    ASSERT_EQ(result.derivatives.rows(), 2);
    ASSERT_EQ(result.derivatives.columns(), 4);

    EXPECT_DOUBLE_EQ(result.derivatives(0, 0), -0.25);
    EXPECT_DOUBLE_EQ(result.derivatives(0, 1), 0.25);
    EXPECT_DOUBLE_EQ(result.derivatives(0, 2), 0.25);
    EXPECT_DOUBLE_EQ(result.derivatives(0, 3), -0.25);
    EXPECT_DOUBLE_EQ(result.derivatives(1, 0), -0.5);
    EXPECT_DOUBLE_EQ(result.derivatives(1, 1), -0.5);
    EXPECT_DOUBLE_EQ(result.derivatives(1, 2), 0.5);
    EXPECT_DOUBLE_EQ(result.derivatives(1, 3), 0.5);
    EXPECT_DOUBLE_EQ(result.jacobian_determinant, 0.5);
}

TEST(Q4PhysicalDerivatives, ComputesSkewParallelogramAtCenter)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{4.0, 0.0}}, {{5.0, 2.0}}, {{1.0, 2.0}}}};

    const auto result = q4_shape_function_physical_derivatives(coordinates, 0.0, 0.0);

    EXPECT_NEAR(result.derivatives(0, 0), -0.125, 1.0e-15);
    EXPECT_NEAR(result.derivatives(0, 1), 0.125, 1.0e-15);
    EXPECT_NEAR(result.derivatives(0, 2), 0.125, 1.0e-15);
    EXPECT_NEAR(result.derivatives(0, 3), -0.125, 1.0e-15);
    EXPECT_NEAR(result.derivatives(1, 0), -0.1875, 1.0e-15);
    EXPECT_NEAR(result.derivatives(1, 1), -0.3125, 1.0e-15);
    EXPECT_NEAR(result.derivatives(1, 2), 0.1875, 1.0e-15);
    EXPECT_NEAR(result.derivatives(1, 3), 0.3125, 1.0e-15);
    EXPECT_NEAR(result.jacobian_determinant, 2.0, 1.0e-15);
}

TEST(Q4PhysicalDerivatives, ReproduceConstantAndLinearPhysicalFields)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{3.0, 0.5}}, {{2.5, 2.0}}, {{-0.5, 1.5}}}};
    const auto result = q4_shape_function_physical_derivatives(coordinates, -0.3, 0.4);

    double d_dx_sum = 0.0;
    double d_dy_sum = 0.0;
    double dx_dx = 0.0;
    double dy_dx = 0.0;
    double dx_dy = 0.0;
    double dy_dy = 0.0;

    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const double derivative_x = result.derivatives(0, node);
        const double derivative_y = result.derivatives(1, node);
        d_dx_sum += derivative_x;
        d_dy_sum += derivative_y;
        dx_dx += derivative_x * coordinates[node][0];
        dy_dx += derivative_x * coordinates[node][1];
        dx_dy += derivative_y * coordinates[node][0];
        dy_dy += derivative_y * coordinates[node][1];
    }

    EXPECT_NEAR(d_dx_sum, 0.0, 1.0e-14);
    EXPECT_NEAR(d_dy_sum, 0.0, 1.0e-14);
    EXPECT_NEAR(dx_dx, 1.0, 1.0e-14);
    EXPECT_NEAR(dy_dx, 0.0, 1.0e-14);
    EXPECT_NEAR(dx_dy, 0.0, 1.0e-14);
    EXPECT_NEAR(dy_dy, 1.0, 1.0e-14);
}

TEST(Q4PhysicalDerivatives, RejectsInvalidGeometry)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{1.0, 0.0}}, {{2.0, 0.0}}, {{3.0, 0.0}}}};

    EXPECT_THROW(static_cast<void>(q4_shape_function_physical_derivatives(coordinates, 0.0, 0.0)),
                 std::invalid_argument);
}
} // namespace
