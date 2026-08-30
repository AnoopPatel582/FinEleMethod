#include "finelemethod/elements/h8_recovery.hpp"

#include "finelemethod/elements/h8_gauss_quadrature.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::elements::h8_gauss_quadrature_2x2x2;
using finelemethod::elements::H8NodeCoordinates;
using finelemethod::elements::recover_h8_gauss_results;
using finelemethod::math::DenseVector;
using finelemethod::model::IsotropicElasticMaterial;

H8NodeCoordinates unit_box()
{
    return {{{{0.0, 0.0, 0.0}},
             {{1.0, 0.0, 0.0}},
             {{1.0, 1.0, 0.0}},
             {{0.0, 1.0, 0.0}},
             {{0.0, 0.0, 1.0}},
             {{1.0, 0.0, 1.0}},
             {{1.0, 1.0, 1.0}},
             {{0.0, 1.0, 1.0}}}};
}

TEST(H8Recovery, RecoversUniformUniaxialCompressionAtEveryGaussPoint)
{
    const H8NodeCoordinates coordinates = unit_box();
    const IsotropicElasticMaterial material(1, 1000.0, 0.25);
    DenseVector displacements(24);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        displacements[3 * node] = 0.0025 * coordinates[node][0];
        displacements[3 * node + 1] = 0.0025 * coordinates[node][1];
        displacements[3 * node + 2] = -0.01 * coordinates[node][2];
    }

    const auto results = recover_h8_gauss_results(coordinates, material, displacements);
    const auto quadrature = h8_gauss_quadrature_2x2x2();

    constexpr double tolerance = 1.0e-12;
    for (std::size_t point_index = 0; point_index < results.size(); ++point_index)
    {
        EXPECT_DOUBLE_EQ(results[point_index].xi, quadrature[point_index].xi);
        EXPECT_DOUBLE_EQ(results[point_index].eta, quadrature[point_index].eta);
        EXPECT_DOUBLE_EQ(results[point_index].zeta, quadrature[point_index].zeta);
        EXPECT_NEAR(results[point_index].strain[0], 0.0025, tolerance);
        EXPECT_NEAR(results[point_index].strain[1], 0.0025, tolerance);
        EXPECT_NEAR(results[point_index].strain[2], -0.01, tolerance);
        EXPECT_NEAR(results[point_index].strain[3], 0.0, tolerance);
        EXPECT_NEAR(results[point_index].strain[4], 0.0, tolerance);
        EXPECT_NEAR(results[point_index].strain[5], 0.0, tolerance);
        EXPECT_NEAR(results[point_index].stress[0], 0.0, tolerance);
        EXPECT_NEAR(results[point_index].stress[1], 0.0, tolerance);
        EXPECT_NEAR(results[point_index].stress[2], -10.0, tolerance);
        EXPECT_NEAR(results[point_index].stress[3], 0.0, tolerance);
        EXPECT_NEAR(results[point_index].stress[4], 0.0, tolerance);
        EXPECT_NEAR(results[point_index].stress[5], 0.0, tolerance);
    }
}

TEST(H8Recovery, RejectsIncorrectLocalDisplacementCount)
{
    const IsotropicElasticMaterial material(1, 1000.0, 0.25);

    EXPECT_THROW(static_cast<void>(recover_h8_gauss_results(unit_box(), material, DenseVector(23))),
                 std::invalid_argument);
}

TEST(H8Recovery, RejectsInvalidElementGeometry)
{
    H8NodeCoordinates coordinates = unit_box();
    for (auto &coordinate : coordinates)
    {
        coordinate[2] = 0.0;
    }
    const IsotropicElasticMaterial material(1, 1000.0, 0.25);

    EXPECT_THROW(
        static_cast<void>(recover_h8_gauss_results(coordinates, material, DenseVector(24))),
        std::invalid_argument);
}
} // namespace
