#pragma once

#include "finelemethod/elements/q4_jacobian.hpp"
#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

namespace finelemethod::elements
{
inline constexpr double default_reduced_integration_stabilization = 0.05;

// One-point CPS4R stiffness with stiffness-based hourglass stabilization.
// The stabilization fraction scales only the modes omitted by one-point
// integration; constant-strain behavior remains exact.
[[nodiscard]] math::DenseMatrix q4_reduced_plane_stress_stiffness_matrix(
    const Q4NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material,
    double thickness, double stabilization_fraction = default_reduced_integration_stabilization);
} // namespace finelemethod::elements
