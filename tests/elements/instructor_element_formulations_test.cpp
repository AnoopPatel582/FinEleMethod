#include "finelemethod/elements/h8_reduced_stiffness.hpp"
#include "finelemethod/elements/h8_stiffness.hpp"
#include "finelemethod/elements/q4_plane_stress_stiffness.hpp"
#include "finelemethod/elements/q4_reduced_stiffness.hpp"
#include "finelemethod/elements/t3.hpp"
#include "finelemethod/elements/t4.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::elements::H8NodeCoordinates;
using finelemethod::elements::Q4NodeCoordinates;
using finelemethod::elements::T3Edge;
using finelemethod::elements::T3NodeCoordinates;
using finelemethod::elements::T4Face;
using finelemethod::elements::T4NodeCoordinates;
using finelemethod::math::DenseVector;
using finelemethod::model::IsotropicElasticMaterial;

TEST(T3Formulation, ReproducesConstantPlaneStressStrainEnergy)
{
    const T3NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 0.0}}, {{0.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    const double thickness = 0.5;
    const auto stiffness =
        finelemethod::elements::t3_plane_stress_stiffness_matrix(coordinates, material, thickness);
    DenseVector displacement(6);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const double x = coordinates[node][0];
        const double y = coordinates[node][1];
        displacement[2 * node] = 0.01 * x + 0.02 * y;
        displacement[2 * node + 1] = -0.03 * x + 0.04 * y;
    }

    DenseVector strain(3);
    strain[0] = 0.01;
    strain[1] = 0.04;
    strain[2] = -0.01;
    const DenseVector stress =
        finelemethod::mechanics::plane_stress_constitutive_matrix(material) * strain;
    const double expected = 1.0 * thickness * finelemethod::math::dot(strain, stress);

    EXPECT_NEAR(finelemethod::math::dot(displacement, stiffness * displacement), expected, 1.0e-12);
}

TEST(T3Formulation, ConvertsUniformEdgePressureToConsistentNodalForces)
{
    const T3NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 0.0}}, {{0.0, 1.0}}}};
    const DenseVector load = finelemethod::elements::t3_uniform_edge_pressure_load(
        coordinates, T3Edge::edge_1, 10.0, 0.5);

    EXPECT_DOUBLE_EQ(load[1], 5.0);
    EXPECT_DOUBLE_EQ(load[3], 5.0);
    EXPECT_DOUBLE_EQ(load[0] + load[2] + load[4], 0.0);
    EXPECT_DOUBLE_EQ(load[1] + load[3] + load[5], 10.0);
}

TEST(T3Formulation, RecoversAffineStrainOnDistortedGeometry)
{
    const T3NodeCoordinates coordinates{{{{0.2, -0.1}}, {{2.3, 0.4}}, {{0.5, 1.8}}}};
    const auto strain_displacement =
        finelemethod::elements::t3_strain_displacement_matrix(coordinates);
    DenseVector displacement(6);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const double x = coordinates[node][0];
        const double y = coordinates[node][1];
        displacement[2 * node] = 0.01 * x + 0.02 * y + 0.4;
        displacement[2 * node + 1] = -0.03 * x + 0.04 * y - 0.2;
    }
    const DenseVector strain = strain_displacement.matrix * displacement;
    EXPECT_NEAR(strain[0], 0.01, 1.0e-14);
    EXPECT_NEAR(strain[1], 0.04, 1.0e-14);
    EXPECT_NEAR(strain[2], -0.01, 1.0e-14);
}

TEST(T3Formulation, RejectsClockwiseOrDegenerateConnectivity)
{
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    const T3NodeCoordinates clockwise{{{{0.0, 0.0}}, {{0.0, 1.0}}, {{2.0, 0.0}}}};
    EXPECT_THROW(static_cast<void>(finelemethod::elements::t3_plane_stress_stiffness_matrix(
                     clockwise, material, 1.0)),
                 std::invalid_argument);
}

TEST(T4Formulation, ReproducesConstantThreeDimensionalStrainEnergy)
{
    const T4NodeCoordinates coordinates{
        {{{0.0, 0.0, 0.0}}, {{1.0, 0.0, 0.0}}, {{0.0, 1.0, 0.0}}, {{0.0, 0.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    const auto stiffness = finelemethod::elements::t4_stiffness_matrix(coordinates, material);
    DenseVector displacement(12);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const double x = coordinates[node][0];
        const double y = coordinates[node][1];
        const double z = coordinates[node][2];
        displacement[3 * node] = 0.01 * x + 0.02 * y - 0.03 * z;
        displacement[3 * node + 1] = 0.04 * x + 0.05 * y + 0.06 * z;
        displacement[3 * node + 2] = -0.07 * x + 0.08 * y + 0.09 * z;
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
    const double expected = finelemethod::math::dot(strain, stress) / 6.0;

    EXPECT_NEAR(finelemethod::math::dot(displacement, stiffness * displacement), expected, 1.0e-12);
}

TEST(T4Formulation, ConvertsUniformFacePressureToConsistentNodalForces)
{
    const T4NodeCoordinates coordinates{
        {{{0.0, 0.0, 0.0}}, {{1.0, 0.0, 0.0}}, {{0.0, 1.0, 0.0}}, {{0.0, 0.0, 1.0}}}};
    const DenseVector load =
        finelemethod::elements::t4_uniform_face_pressure_load(coordinates, T4Face::face_1, 12.0);

    EXPECT_DOUBLE_EQ(load[2], 2.0);
    EXPECT_DOUBLE_EQ(load[5], 2.0);
    EXPECT_DOUBLE_EQ(load[8], 2.0);
    EXPECT_DOUBLE_EQ(load[11], 0.0);
}

TEST(T4Formulation, RecoversAffineStrainOnDistortedGeometry)
{
    const T4NodeCoordinates coordinates{
        {{{0.2, -0.1, 0.3}}, {{2.2, 0.2, 0.5}}, {{0.6, 1.4, 0.4}}, {{0.4, 0.2, 1.5}}}};
    const auto strain_displacement =
        finelemethod::elements::t4_strain_displacement_matrix(coordinates);
    DenseVector displacement(12);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const double x = coordinates[node][0];
        const double y = coordinates[node][1];
        const double z = coordinates[node][2];
        displacement[3 * node] = 0.01 * x + 0.02 * y - 0.03 * z + 0.5;
        displacement[3 * node + 1] = 0.04 * x + 0.05 * y + 0.06 * z - 0.2;
        displacement[3 * node + 2] = -0.07 * x + 0.08 * y + 0.09 * z + 0.1;
    }
    const DenseVector strain = strain_displacement.matrix * displacement;
    constexpr std::array<double, 6> expected{{0.01, 0.05, 0.09, 0.06, 0.14, -0.10}};
    for (std::size_t component = 0; component < expected.size(); ++component)
    {
        EXPECT_NEAR(strain[component], expected[component], 1.0e-13);
    }
}

TEST(Q4ReducedFormulation, PreservesConstantStrainAndStabilizesHourglassMode)
{
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{1.0, 0.0}}, {{1.0, 1.0}}, {{0.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 1000.0, 0.25);
    const auto full =
        finelemethod::elements::q4_plane_stress_stiffness_matrix(coordinates, material, 1.0);
    const auto reduced = finelemethod::elements::q4_reduced_plane_stress_stiffness_matrix(
        coordinates, material, 1.0);

    DenseVector affine(8);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        affine[2 * node] = 0.02 * coordinates[node][0];
        affine[2 * node + 1] = -0.01 * coordinates[node][1];
    }
    EXPECT_NEAR(finelemethod::math::dot(affine, reduced * affine),
                finelemethod::math::dot(affine, full * affine), 1.0e-12);

    DenseVector hourglass(8);
    hourglass[0] = 1.0;
    hourglass[2] = -1.0;
    hourglass[4] = 1.0;
    hourglass[6] = -1.0;
    const double full_energy = finelemethod::math::dot(hourglass, full * hourglass);
    const double reduced_energy = finelemethod::math::dot(hourglass, reduced * hourglass);
    EXPECT_GT(reduced_energy, 0.0);
    EXPECT_NEAR(reduced_energy, 0.05 * full_energy, 1.0e-11);
}

TEST(H8ReducedFormulation, PreservesConstantStrainAndStabilizesHourglassMode)
{
    const H8NodeCoordinates coordinates{{{{0.0, 0.0, 0.0}},
                                         {{1.0, 0.0, 0.0}},
                                         {{1.0, 1.0, 0.0}},
                                         {{0.0, 1.0, 0.0}},
                                         {{0.0, 0.0, 1.0}},
                                         {{1.0, 0.0, 1.0}},
                                         {{1.0, 1.0, 1.0}},
                                         {{0.0, 1.0, 1.0}}}};
    const IsotropicElasticMaterial material(1, 1000.0, 0.25);
    const auto full = finelemethod::elements::h8_stiffness_matrix(coordinates, material);
    const auto reduced = finelemethod::elements::h8_reduced_stiffness_matrix(coordinates, material);

    DenseVector affine(24);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        affine[3 * node] = 0.02 * coordinates[node][0];
        affine[3 * node + 1] = -0.01 * coordinates[node][1];
        affine[3 * node + 2] = 0.03 * coordinates[node][2];
    }
    EXPECT_NEAR(finelemethod::math::dot(affine, reduced * affine),
                finelemethod::math::dot(affine, full * affine), 1.0e-12);

    DenseVector hourglass(24);
    constexpr std::array<double, 8> signs{{1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0}};
    for (std::size_t node = 0; node < signs.size(); ++node)
    {
        hourglass[3 * node] = signs[node];
    }
    const double full_energy = finelemethod::math::dot(hourglass, full * hourglass);
    const double reduced_energy = finelemethod::math::dot(hourglass, reduced * hourglass);
    EXPECT_GT(reduced_energy, 0.0);
    EXPECT_NEAR(reduced_energy, 0.05 * full_energy, 1.0e-11);
}
} // namespace
