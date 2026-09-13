#pragma once

#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

#include <array>

namespace finelemethod::elements
{
using T3NodeCoordinates = std::array<std::array<double, 2>, 3>;

struct T3StrainDisplacement
{
    // Maps [u1, v1, u2, v2, u3, v3] to
    // [epsilon_x, epsilon_y, gamma_xy].
    math::DenseMatrix matrix;
    double area;
};

enum class T3Edge
{
    edge_1,
    edge_2,
    edge_3,
};

[[nodiscard]] T3StrainDisplacement t3_strain_displacement_matrix(
    const T3NodeCoordinates &coordinates);

[[nodiscard]] math::DenseMatrix t3_plane_stress_stiffness_matrix(
    const T3NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material,
    double thickness);

// Positive pressure acts inward. Edge numbering follows ABAQUS CPS3:
// S1=(1,2), S2=(2,3), S3=(3,1).
[[nodiscard]] math::DenseVector t3_uniform_edge_pressure_load(const T3NodeCoordinates &coordinates,
                                                              T3Edge edge, double pressure,
                                                              double thickness);
} // namespace finelemethod::elements
