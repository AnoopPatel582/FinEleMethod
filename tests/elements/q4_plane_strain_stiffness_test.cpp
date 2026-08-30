#include "finelemethod/elements/q4_plane_strain_stiffness.hpp"

#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::elements::q4_plane_strain_stiffness_matrix;
using finelemethod::elements::Q4NodeCoordinates;
using finelemethod::math::DenseVector;
using finelemethod::model::IsotropicElasticMaterial;

TEST(Q4PlaneStrainStiffness, ProducesSymmetricEightByEightMatrix)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 0.0}}, {{2.0, 1.0}}, {{0.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 210.0e9, 0.3);

    const auto stiffness = q4_plane_strain_stiffness_matrix(coordinates, material, 0.02);

    ASSERT_EQ(stiffness.rows(), 8U);
    ASSERT_EQ(stiffness.columns(), 8U);
    for (std::size_t row = 0; row < stiffness.rows(); ++row)
    {
        for (std::size_t column = 0; column < stiffness.columns(); ++column)
        {
            EXPECT_NEAR(stiffness(row, column), stiffness(column, row), 1.0e-5);
        }
    }
}

TEST(Q4PlaneStrainStiffness, HasZeroForceForRigidBodyModes)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{3.0, 0.5}}, {{2.5, 2.0}}, {{-0.5, 1.5}}}};
    const IsotropicElasticMaterial material(1, 70.0e9, 0.25);
    const auto stiffness = q4_plane_strain_stiffness_matrix(coordinates, material, 0.01);
    std::array<DenseVector, 3> modes{DenseVector(8), DenseVector(8), DenseVector(8)};
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        modes[0][2 * node] = 1.0;
        modes[1][2 * node + 1] = 1.0;
        modes[2][2 * node] = -coordinates[node][1];
        modes[2][2 * node + 1] = coordinates[node][0];
    }

    for (const DenseVector &mode : modes)
    {
        const DenseVector force = stiffness * mode;
        for (std::size_t dof = 0; dof < force.size(); ++dof)
        {
            EXPECT_NEAR(force[dof], 0.0, 1.0e-6);
        }
    }
}

TEST(Q4PlaneStrainStiffness, ReproducesConstantStrainEnergy)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 0.0}}, {{2.0, 1.0}}, {{0.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    constexpr double thickness = 0.5;
    const auto stiffness = q4_plane_strain_stiffness_matrix(coordinates, material, thickness);
    DenseVector displacements(8);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const double x = coordinates[node][0];
        const double y = coordinates[node][1];
        displacements[2 * node] = 0.01 * x + 0.02 * y;
        displacements[2 * node + 1] = -0.03 * x + 0.04 * y;
    }

    DenseVector strain(3);
    strain[0] = 0.01;
    strain[1] = 0.04;
    strain[2] = -0.01;
    const DenseVector stress =
        finelemethod::mechanics::plane_strain_constitutive_matrix(material) * strain;
    const double finite_element_energy_twice =
        finelemethod::math::dot(displacements, stiffness * displacements);
    const double expected_energy_twice = 2.0 * thickness * finelemethod::math::dot(strain, stress);

    EXPECT_NEAR(finite_element_energy_twice, expected_energy_twice, 1.0e-12);
}

TEST(Q4PlaneStrainStiffness, ScalesLinearlyWithThickness)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{1.0, 0.0}}, {{1.0, 1.0}}, {{0.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 1000.0, 0.2);
    const auto thin = q4_plane_strain_stiffness_matrix(coordinates, material, 0.1);
    const auto thick = q4_plane_strain_stiffness_matrix(coordinates, material, 0.3);

    for (std::size_t row = 0; row < thin.rows(); ++row)
    {
        for (std::size_t column = 0; column < thin.columns(); ++column)
        {
            EXPECT_NEAR(thick(row, column), 3.0 * thin(row, column), 1.0e-12);
        }
    }
}

TEST(Q4PlaneStrainStiffness, RejectsInvalidThickness)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{1.0, 0.0}}, {{1.0, 1.0}}, {{0.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 1000.0, 0.2);

    EXPECT_THROW(static_cast<void>(q4_plane_strain_stiffness_matrix(coordinates, material, 0.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(q4_plane_strain_stiffness_matrix(coordinates, material, -0.1)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(q4_plane_strain_stiffness_matrix(
                     coordinates, material, std::numeric_limits<double>::infinity())),
                 std::invalid_argument);
}
} // namespace
