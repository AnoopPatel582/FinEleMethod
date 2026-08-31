#pragma once

#include "finelemethod/elements/h8_jacobian.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

#include <array>

namespace finelemethod::elements
{
struct H8PointResult
{
    double xi;
    double eta;
    double zeta;
    // Engineering strain order:
    // [epsilon_x, epsilon_y, epsilon_z, gamma_xy, gamma_yz, gamma_zx].
    std::array<double, 6> strain;
    // Stress order:
    // [sigma_x, sigma_y, sigma_z, tau_xy, tau_yz, tau_zx].
    std::array<double, 6> stress;
    double von_mises;
    // Descending order [sigma_1, sigma_2, sigma_3].
    std::array<double, 3> principal_stresses;
};

using H8GaussResults = std::array<H8PointResult, 8>;

// Recovers strain and stress at the eight 2x2x2 Gauss points from local nodal
// displacements ordered as [u1, v1, w1, ..., u8, v8, w8].
[[nodiscard]] H8GaussResults recover_h8_gauss_results(
    const H8NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material,
    const math::DenseVector &local_displacements);
} // namespace finelemethod::elements
