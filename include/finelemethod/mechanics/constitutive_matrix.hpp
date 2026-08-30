#pragma once

#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

namespace finelemethod::mechanics
{
// Uses the engineering strain order [epsilon_x, epsilon_y, gamma_xy] and
// returns stresses in the order [sigma_x, sigma_y, tau_xy].
[[nodiscard]] math::DenseMatrix plane_stress_constitutive_matrix(
    const model::IsotropicElasticMaterial &material);

// Uses the engineering strain order [epsilon_x, epsilon_y, gamma_xy] with
// epsilon_z constrained to zero, and returns [sigma_x, sigma_y, tau_xy].
[[nodiscard]] math::DenseMatrix plane_strain_constitutive_matrix(
    const model::IsotropicElasticMaterial &material);
} // namespace finelemethod::mechanics
