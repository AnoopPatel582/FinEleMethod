#include "finelemethod/elements/q4_plane_stress_stiffness.hpp"

#include "finelemethod/math/dense_vector.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::elements::q4_plane_stress_stiffness_matrix;
using finelemethod::elements::Q4NodeCoordinates;
using finelemethod::math::DenseVector;
using finelemethod::model::IsotropicElasticMaterial;

TEST(Q4PlaneStressStiffness, ProducesSymmetricEightByEightMatrix)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 0.0}}, {{2.0, 1.0}}, {{0.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 210.0e9, 0.3);

    const auto stiffness = q4_plane_stress_stiffness_matrix(coordinates, material, 0.02);

    ASSERT_EQ(stiffness.rows(), 8);
    ASSERT_EQ(stiffness.columns(), 8);
    for (std::size_t row = 0; row < stiffness.rows(); ++row)
    {
        for (std::size_t column = 0; column < stiffness.columns(); ++column)
        {
            EXPECT_NEAR(stiffness(row, column), stiffness(column, row), 1.0e-5);
        }
    }
}

TEST(Q4PlaneStressStiffness, HasZeroForceForRigidBodyModes)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{3.0, 0.5}}, {{2.5, 2.0}}, {{-0.5, 1.5}}}};
    const IsotropicElasticMaterial material(1, 70.0e9, 0.25);
    const auto stiffness = q4_plane_stress_stiffness_matrix(coordinates, material, 0.01);

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

TEST(Q4PlaneStressStiffness, ReproducesConstantStrainEnergy)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 0.0}}, {{2.0, 1.0}}, {{0.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    const double thickness = 0.5;
    const auto stiffness = q4_plane_stress_stiffness_matrix(coordinates, material, thickness);
    DenseVector displacements(8);

    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const double x = coordinates[node][0];
        const double y = coordinates[node][1];
        displacements[2 * node] = 0.01 * x + 0.02 * y;
        displacements[2 * node + 1] = -0.03 * x + 0.04 * y;
    }

    const double finite_element_energy_twice =
        finelemethod::math::dot(displacements, stiffness * displacements);
    const double epsilon_x = 0.01;
    const double epsilon_y = 0.04;
    const double gamma_xy = -0.01;
    const double factor =
        material.youngs_modulus() / (1.0 - material.poisson_ratio() * material.poisson_ratio());
    const double stress_x = factor * (epsilon_x + material.poisson_ratio() * epsilon_y);
    const double stress_y = factor * (material.poisson_ratio() * epsilon_x + epsilon_y);
    const double shear_stress = factor * (1.0 - material.poisson_ratio()) * gamma_xy / 2.0;
    const double area = 2.0;
    const double expected_energy_twice =
        area * thickness * (epsilon_x * stress_x + epsilon_y * stress_y + gamma_xy * shear_stress);

    EXPECT_NEAR(finite_element_energy_twice, expected_energy_twice, 1.0e-12);
}

TEST(Q4PlaneStressStiffness, ScalesLinearlyWithThickness)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{1.0, 0.0}}, {{1.0, 1.0}}, {{0.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 1000.0, 0.2);
    const auto thin = q4_plane_stress_stiffness_matrix(coordinates, material, 0.1);
    const auto thick = q4_plane_stress_stiffness_matrix(coordinates, material, 0.3);

    for (std::size_t row = 0; row < thin.rows(); ++row)
    {
        for (std::size_t column = 0; column < thin.columns(); ++column)
        {
            EXPECT_NEAR(thick(row, column), 3.0 * thin(row, column), 1.0e-12);
        }
    }
}

TEST(Q4PlaneStressStiffness, RejectsInvalidThickness)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{1.0, 0.0}}, {{1.0, 1.0}}, {{0.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 1000.0, 0.2);

    EXPECT_THROW(static_cast<void>(q4_plane_stress_stiffness_matrix(coordinates, material, 0.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(q4_plane_stress_stiffness_matrix(coordinates, material, -0.1)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(q4_plane_stress_stiffness_matrix(
                     coordinates, material, std::numeric_limits<double>::infinity())),
                 std::invalid_argument);
}
} // namespace
