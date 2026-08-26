#include "finelemethod/elements/q4_strain_displacement.hpp"

#include "finelemethod/math/dense_vector.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>

namespace
{
using finelemethod::elements::q4_strain_displacement_matrix;
using finelemethod::elements::Q4NodeCoordinates;
using finelemethod::math::DenseVector;

TEST(Q4StrainDisplacement, BuildsMatrixForAxisAlignedRectangle)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 0.0}}, {{2.0, 1.0}}, {{0.0, 1.0}}}};

    const auto result = q4_strain_displacement_matrix(coordinates, 0.0, 0.0);
    const std::array<std::array<double, 8>, 3> expected{{
        {{-0.25, 0.0, 0.25, 0.0, 0.25, 0.0, -0.25, 0.0}},
        {{0.0, -0.5, 0.0, -0.5, 0.0, 0.5, 0.0, 0.5}},
        {{-0.5, -0.25, -0.5, 0.25, 0.5, 0.25, 0.5, -0.25}},
    }};

    ASSERT_EQ(result.matrix.rows(), 3);
    ASSERT_EQ(result.matrix.columns(), 8);
    for (std::size_t row = 0; row < expected.size(); ++row)
    {
        for (std::size_t column = 0; column < expected[row].size(); ++column)
        {
            EXPECT_DOUBLE_EQ(result.matrix(row, column), expected[row][column]);
        }
    }
    EXPECT_DOUBLE_EQ(result.jacobian_determinant, 0.5);
}

TEST(Q4StrainDisplacement, ReproducesConstantStrainField)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{3.0, 0.5}}, {{2.5, 2.0}}, {{-0.5, 1.5}}}};
    const auto result = q4_strain_displacement_matrix(coordinates, -0.3, 0.4);
    DenseVector displacements(8);

    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const double x = coordinates[node][0];
        const double y = coordinates[node][1];
        displacements[2 * node] = 0.01 * x + 0.02 * y + 0.7;
        displacements[2 * node + 1] = -0.03 * x + 0.04 * y - 0.2;
    }

    const DenseVector strain = result.matrix * displacements;

    ASSERT_EQ(strain.size(), 3);
    EXPECT_NEAR(strain[0], 0.01, 1.0e-14);
    EXPECT_NEAR(strain[1], 0.04, 1.0e-14);
    EXPECT_NEAR(strain[2], -0.01, 1.0e-14);
}

TEST(Q4StrainDisplacement, RejectsInvalidGeometry)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{1.0, 0.0}}, {{2.0, 0.0}}, {{3.0, 0.0}}}};

    EXPECT_THROW(static_cast<void>(q4_strain_displacement_matrix(coordinates, 0.0, 0.0)),
                 std::invalid_argument);
}
} // namespace
