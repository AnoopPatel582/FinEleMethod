#pragma once

#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

#include <array>

namespace finelemethod::elements
{
using T4NodeCoordinates = std::array<std::array<double, 3>, 4>;

struct T4StrainDisplacement
{
    // Maps [u1, v1, w1, ..., u4, v4, w4] to
    // [epsilon_x, epsilon_y, epsilon_z, gamma_xy, gamma_yz, gamma_zx].
    math::DenseMatrix matrix;
    double volume;
};

enum class T4Face
{
    face_1,
    face_2,
    face_3,
    face_4,
};

[[nodiscard]] T4StrainDisplacement t4_strain_displacement_matrix(
    const T4NodeCoordinates &coordinates);

[[nodiscard]] math::DenseMatrix t4_stiffness_matrix(
    const T4NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material);

// Returns consistent nodal forces for uniform inward pressure.
[[nodiscard]] math::DenseVector t4_uniform_face_pressure_load(const T4NodeCoordinates &coordinates,
                                                              T4Face face, double pressure);
} // namespace finelemethod::elements
