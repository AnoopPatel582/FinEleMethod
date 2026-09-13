#pragma once

#include "finelemethod/elements/h8_jacobian.hpp"
#include "finelemethod/elements/q4_reduced_stiffness.hpp"
#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

namespace finelemethod::elements
{
// One-point C3D8R stiffness with stiffness-based hourglass stabilization.
// The stabilization fraction scales only the modes omitted by one-point
// integration; constant-strain behavior remains exact.
[[nodiscard]] math::DenseMatrix h8_reduced_stiffness_matrix(
    const H8NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material,
    double stabilization_fraction = default_reduced_integration_stabilization);
} // namespace finelemethod::elements
