#include "finelemethod/elements/q4_plane_stress_recovery.hpp"

#include "finelemethod/elements/q4_gauss_quadrature.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::elements::q4_gauss_quadrature_2x2;
using finelemethod::elements::Q4NodeCoordinates;
using finelemethod::elements::recover_q4_plane_stress_gauss_results;
using finelemethod::math::DenseVector;
using finelemethod::model::IsotropicElasticMaterial;

TEST(Q4PlaneStressRecovery, RecoversUniformUniaxialStateAtEveryGaussPoint)
{
    const Q4NodeCoordinates coordinates{{
        {{0.0, 0.0}},
        {{2.0, 0.0}},
        {{2.0, 1.0}},
        {{0.0, 1.0}},
    }};
    const IsotropicElasticMaterial material(1, 1000.0, 0.25);
    DenseVector displacements(8);
    displacements[0] = 0.0;
    displacements[1] = 0.0;
    displacements[2] = 0.02;
    displacements[3] = 0.0;
    displacements[4] = 0.02;
    displacements[5] = -0.0025;
    displacements[6] = 0.0;
    displacements[7] = -0.0025;

    const auto results =
        recover_q4_plane_stress_gauss_results(coordinates, material, displacements);
    const auto quadrature = q4_gauss_quadrature_2x2();

    constexpr double tolerance = 1.0e-12;
    for (std::size_t point_index = 0; point_index < results.size(); ++point_index)
    {
        EXPECT_DOUBLE_EQ(results[point_index].xi, quadrature[point_index].xi);
        EXPECT_DOUBLE_EQ(results[point_index].eta, quadrature[point_index].eta);
        EXPECT_NEAR(results[point_index].strain[0], 0.01, tolerance);
        EXPECT_NEAR(results[point_index].strain[1], -0.0025, tolerance);
        EXPECT_NEAR(results[point_index].strain[2], 0.0, tolerance);
        EXPECT_NEAR(results[point_index].stress[0], 10.0, tolerance);
        EXPECT_NEAR(results[point_index].stress[1], 0.0, tolerance);
        EXPECT_NEAR(results[point_index].stress[2], 0.0, tolerance);
        EXPECT_NEAR(results[point_index].von_mises, 10.0, tolerance);
        EXPECT_NEAR(results[point_index].principal_stresses[0], 10.0, tolerance);
        EXPECT_NEAR(results[point_index].principal_stresses[1], 0.0, tolerance);
        EXPECT_NEAR(results[point_index].principal_stresses[2], 0.0, tolerance);
    }
}

TEST(Q4PlaneStressRecovery, RejectsIncorrectLocalDisplacementCount)
{
    const Q4NodeCoordinates coordinates{{
        {{0.0, 0.0}},
        {{1.0, 0.0}},
        {{1.0, 1.0}},
        {{0.0, 1.0}},
    }};
    const IsotropicElasticMaterial material(1, 1000.0, 0.25);

    EXPECT_THROW(static_cast<void>(
                     recover_q4_plane_stress_gauss_results(coordinates, material, DenseVector(7))),
                 std::invalid_argument);
}

TEST(Q4PlaneStressRecovery, RejectsInvalidElementGeometry)
{
    const Q4NodeCoordinates clockwise_coordinates{{
        {{0.0, 0.0}},
        {{0.0, 1.0}},
        {{1.0, 1.0}},
        {{1.0, 0.0}},
    }};
    const IsotropicElasticMaterial material(1, 1000.0, 0.25);

    EXPECT_THROW(static_cast<void>(recover_q4_plane_stress_gauss_results(clockwise_coordinates,
                                                                         material, DenseVector(8))),
                 std::invalid_argument);
}
} // namespace
