#include "finelemethod/elements/h8_stiffness.hpp"

#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::elements::h8_stiffness_matrix;
using finelemethod::elements::H8NodeCoordinates;
using finelemethod::math::DenseVector;
using finelemethod::model::IsotropicElasticMaterial;

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

TEST(H8Stiffness, ProducesSymmetricTwentyFourByTwentyFourMatrix)
{
    const IsotropicElasticMaterial material(1, 210.0e9, 0.3);

    const auto stiffness = h8_stiffness_matrix(axis_aligned_box(), material);

    ASSERT_EQ(stiffness.rows(), 24);
    ASSERT_EQ(stiffness.columns(), 24);
    for (std::size_t row = 0; row < stiffness.rows(); ++row)
    {
        for (std::size_t column = 0; column < stiffness.columns(); ++column)
        {
            const double scale =
                std::max({1.0, std::abs(stiffness(row, column)), std::abs(stiffness(column, row))});
            EXPECT_NEAR(stiffness(row, column), stiffness(column, row), 1.0e-14 * scale);
        }
    }
}

TEST(H8Stiffness, HasZeroForceForSixRigidBodyModes)
{
    const H8NodeCoordinates coordinates = axis_aligned_box();
    const IsotropicElasticMaterial material(1, 1000.0, 0.25);
    const auto stiffness = h8_stiffness_matrix(coordinates, material);
    std::array<DenseVector, 6> modes{DenseVector(24), DenseVector(24), DenseVector(24),
                                     DenseVector(24), DenseVector(24), DenseVector(24)};

    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const double x = coordinates[node][0];
        const double y = coordinates[node][1];
        const double z = coordinates[node][2];
        modes[0][3 * node] = 1.0;
        modes[1][3 * node + 1] = 1.0;
        modes[2][3 * node + 2] = 1.0;
        modes[3][3 * node + 1] = -z;
        modes[3][3 * node + 2] = y;
        modes[4][3 * node] = z;
        modes[4][3 * node + 2] = -x;
        modes[5][3 * node] = -y;
        modes[5][3 * node + 1] = x;
    }

    for (const DenseVector &mode : modes)
    {
        const DenseVector force = stiffness * mode;
        for (std::size_t dof = 0; dof < force.size(); ++dof)
        {
            EXPECT_NEAR(force[dof], 0.0, 1.0e-10);
        }
    }
}

TEST(H8Stiffness, ReproducesConstantStrainEnergy)
{
    const H8NodeCoordinates coordinates = axis_aligned_box();
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    const auto stiffness = h8_stiffness_matrix(coordinates, material);
    DenseVector displacements(24);

    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const double x = coordinates[node][0];
        const double y = coordinates[node][1];
        const double z = coordinates[node][2];
        displacements[3 * node] = 0.01 * x + 0.02 * y - 0.03 * z;
        displacements[3 * node + 1] = 0.04 * x + 0.05 * y + 0.06 * z;
        displacements[3 * node + 2] = -0.07 * x + 0.08 * y + 0.09 * z;
    }

    DenseVector strain(6);
    strain[0] = 0.01;
    strain[1] = 0.05;
    strain[2] = 0.09;
    strain[3] = 0.06;
    strain[4] = 0.14;
    strain[5] = -0.10;
    const DenseVector stress =
        finelemethod::mechanics::solid_isotropic_constitutive_matrix(material) * strain;
    const double finite_element_energy_twice =
        finelemethod::math::dot(displacements, stiffness * displacements);
    const double expected_energy_twice = 48.0 * finelemethod::math::dot(strain, stress);

    EXPECT_NEAR(finite_element_energy_twice, expected_energy_twice, 1.0e-10);
}

TEST(H8Stiffness, ScalesLinearlyWithYoungsModulusAndRejectsInvalidGeometry)
{
    const H8NodeCoordinates coordinates = axis_aligned_box();
    const auto low = h8_stiffness_matrix(coordinates, IsotropicElasticMaterial(1, 1000.0, 0.2));
    const auto high = h8_stiffness_matrix(coordinates, IsotropicElasticMaterial(1, 3000.0, 0.2));

    for (std::size_t row = 0; row < low.rows(); ++row)
    {
        for (std::size_t column = 0; column < low.columns(); ++column)
        {
            EXPECT_NEAR(high(row, column), 3.0 * low(row, column), 1.0e-10);
        }
    }

    H8NodeCoordinates degenerate = coordinates;
    for (auto &coordinate : degenerate)
    {
        coordinate[2] = 0.0;
    }
    EXPECT_THROW(static_cast<void>(
                     h8_stiffness_matrix(degenerate, IsotropicElasticMaterial(1, 1000.0, 0.2))),
                 std::invalid_argument);
}
} // namespace
