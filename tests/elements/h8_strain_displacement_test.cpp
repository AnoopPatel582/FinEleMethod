#include "finelemethod/elements/h8_strain_displacement.hpp"

#include "finelemethod/math/dense_vector.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::elements::h8_strain_displacement_matrix;
using finelemethod::elements::H8NodeCoordinates;
using finelemethod::math::DenseVector;

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

TEST(H8StrainDisplacement, BuildsMatrixForAxisAlignedBox)
{
    const auto result = h8_strain_displacement_matrix(axis_aligned_box(), 0.0, 0.0, 0.0);

    ASSERT_EQ(result.matrix.rows(), 6);
    ASSERT_EQ(result.matrix.columns(), 24);
    EXPECT_DOUBLE_EQ(result.matrix(0, 0), -0.125);
    EXPECT_DOUBLE_EQ(result.matrix(1, 1), -0.0625);
    EXPECT_DOUBLE_EQ(result.matrix(2, 2), -1.0 / 24.0);
    EXPECT_DOUBLE_EQ(result.matrix(3, 0), -0.0625);
    EXPECT_DOUBLE_EQ(result.matrix(3, 1), -0.125);
    EXPECT_DOUBLE_EQ(result.matrix(4, 1), -1.0 / 24.0);
    EXPECT_DOUBLE_EQ(result.matrix(4, 2), -0.0625);
    EXPECT_DOUBLE_EQ(result.matrix(5, 0), -1.0 / 24.0);
    EXPECT_DOUBLE_EQ(result.matrix(5, 2), -0.125);
    EXPECT_DOUBLE_EQ(result.matrix(0, 1), 0.0);
    EXPECT_DOUBLE_EQ(result.matrix(0, 2), 0.0);
    EXPECT_DOUBLE_EQ(result.jacobian_determinant, 6.0);
}

TEST(H8StrainDisplacement, ReproducesConstantThreeDimensionalStrainField)
{
    const H8NodeCoordinates coordinates{{{{0.0, 0.0, 0.0}},
                                         {{2.0, 0.0, 0.0}},
                                         {{3.0, 4.0, 0.0}},
                                         {{1.0, 4.0, 0.0}},
                                         {{0.5, 1.0, 6.0}},
                                         {{2.5, 1.0, 6.0}},
                                         {{3.5, 5.0, 6.0}},
                                         {{1.5, 5.0, 6.0}}}};
    const auto result = h8_strain_displacement_matrix(coordinates, -0.3, 0.4, -0.2);
    DenseVector displacements(24);

    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const double x = coordinates[node][0];
        const double y = coordinates[node][1];
        const double z = coordinates[node][2];
        displacements[3 * node] = 0.01 * x + 0.02 * y - 0.03 * z + 0.7;
        displacements[3 * node + 1] = 0.04 * x + 0.05 * y + 0.06 * z - 0.2;
        displacements[3 * node + 2] = -0.07 * x + 0.08 * y + 0.09 * z + 0.4;
    }

    const DenseVector strain = result.matrix * displacements;

    ASSERT_EQ(strain.size(), 6);
    EXPECT_NEAR(strain[0], 0.01, 1.0e-14);
    EXPECT_NEAR(strain[1], 0.05, 1.0e-14);
    EXPECT_NEAR(strain[2], 0.09, 1.0e-14);
    EXPECT_NEAR(strain[3], 0.06, 1.0e-14);
    EXPECT_NEAR(strain[4], 0.14, 1.0e-14);
    EXPECT_NEAR(strain[5], -0.10, 1.0e-14);
}

TEST(H8StrainDisplacement, RejectsInvalidGeometry)
{
    H8NodeCoordinates coordinates = axis_aligned_box();
    for (auto &coordinate : coordinates)
    {
        coordinate[2] = 0.0;
    }

    EXPECT_THROW(static_cast<void>(h8_strain_displacement_matrix(coordinates, 0.0, 0.0, 0.0)),
                 std::invalid_argument);
}
} // namespace
