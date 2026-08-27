#pragma once

#include "finelemethod/elements/q4_jacobian.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

#include <array>

namespace finelemethod::elements
{
struct Q4PlaneStressPointResult
{
    double xi;
    double eta;
    // Engineering strain order: [epsilon_x, epsilon_y, gamma_xy].
    std::array<double, 3> strain;
    // Stress order: [sigma_x, sigma_y, tau_xy].
    std::array<double, 3> stress;
};

using Q4PlaneStressGaussResults = std::array<Q4PlaneStressPointResult, 4>;

// Recovers strain and stress at the four 2x2 Gauss points from local nodal
// displacements ordered as [u1, v1, ..., u4, v4].
[[nodiscard]] Q4PlaneStressGaussResults recover_q4_plane_stress_gauss_results(
    const Q4NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material,
    const math::DenseVector &local_displacements);
} // namespace finelemethod::elements
