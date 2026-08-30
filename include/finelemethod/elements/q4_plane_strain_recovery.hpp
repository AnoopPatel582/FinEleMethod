#pragma once

#include "finelemethod/elements/q4_jacobian.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

#include <array>

namespace finelemethod::elements
{
struct Q4PlaneStrainPointResult
{
    double xi;
    double eta;
    // Engineering strain order: [epsilon_x, epsilon_y, epsilon_z, gamma_xy].
    std::array<double, 4> strain;
    // Stress order: [sigma_x, sigma_y, sigma_z, tau_xy].
    std::array<double, 4> stress;
    double von_mises;
    // Descending order [sigma_1, sigma_2, sigma_3].
    std::array<double, 3> principal_stresses;
};

using Q4PlaneStrainGaussResults = std::array<Q4PlaneStrainPointResult, 4>;

// Recovers strain and stress at the four 2x2 Gauss points from local nodal
// displacements ordered as [u1, v1, ..., u4, v4].
[[nodiscard]] Q4PlaneStrainGaussResults recover_q4_plane_strain_gauss_results(
    const Q4NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material,
    const math::DenseVector &local_displacements);
} // namespace finelemethod::elements
