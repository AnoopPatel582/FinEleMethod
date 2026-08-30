#include "finelemethod/elements/h8_physical_derivatives.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::elements::h8_shape_function_physical_derivatives;
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

TEST(H8PhysicalDerivatives, ComputesAxisAlignedBoxAtCenter)
{
    const auto result = h8_shape_function_physical_derivatives(axis_aligned_box(), 0.0, 0.0, 0.0);

    ASSERT_EQ(result.derivatives.rows(), 3);
    ASSERT_EQ(result.derivatives.columns(), 8);
    const double expected_x[8]{-0.125, 0.125, 0.125, -0.125, -0.125, 0.125, 0.125, -0.125};
    const double expected_y[8]{-0.0625, -0.0625, 0.0625, 0.0625, -0.0625, -0.0625, 0.0625, 0.0625};
    const double expected_z[8]{-1.0 / 24.0, -1.0 / 24.0, -1.0 / 24.0, -1.0 / 24.0,
                               1.0 / 24.0,  1.0 / 24.0,  1.0 / 24.0,  1.0 / 24.0};
    for (std::size_t node = 0; node < 8; ++node)
    {
        EXPECT_DOUBLE_EQ(result.derivatives(0, node), expected_x[node]);
        EXPECT_DOUBLE_EQ(result.derivatives(1, node), expected_y[node]);
        EXPECT_DOUBLE_EQ(result.derivatives(2, node), expected_z[node]);
    }
    EXPECT_DOUBLE_EQ(result.jacobian_determinant, 6.0);
}

TEST(H8PhysicalDerivatives, ReproducesConstantAndLinearPhysicalFields)
{
    const H8NodeCoordinates coordinates{{{{0.0, 0.0, 0.0}},
                                         {{2.0, 0.0, 0.0}},
                                         {{3.0, 4.0, 0.0}},
                                         {{1.0, 4.0, 0.0}},
                                         {{0.5, 1.0, 6.0}},
                                         {{2.5, 1.0, 6.0}},
                                         {{3.5, 5.0, 6.0}},
                                         {{1.5, 5.0, 6.0}}}};
    const auto result = h8_shape_function_physical_derivatives(coordinates, -0.3, 0.4, -0.2);

    for (std::size_t derivative = 0; derivative < 3; ++derivative)
    {
        double constant_derivative = 0.0;
        double coordinate_derivatives[3]{};
        for (std::size_t node = 0; node < coordinates.size(); ++node)
        {
            const double value = result.derivatives(derivative, node);
            constant_derivative += value;
            for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
            {
                coordinate_derivatives[coordinate] += value * coordinates[node][coordinate];
            }
        }

        EXPECT_NEAR(constant_derivative, 0.0, 1.0e-14);
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
        {
            EXPECT_NEAR(coordinate_derivatives[coordinate], derivative == coordinate ? 1.0 : 0.0,
                        1.0e-14);
        }
    }
}

TEST(H8PhysicalDerivatives, RejectsInvalidGeometry)
{
    H8NodeCoordinates coordinates = axis_aligned_box();
    for (auto &coordinate : coordinates)
    {
        coordinate[2] = 0.0;
    }

    EXPECT_THROW(
        static_cast<void>(h8_shape_function_physical_derivatives(coordinates, 0.0, 0.0, 0.0)),
        std::invalid_argument);
}
} // namespace
