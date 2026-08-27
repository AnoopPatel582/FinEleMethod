#pragma once

#include "finelemethod/elements/q4_jacobian.hpp"
#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

namespace finelemethod::elements
{
// Returns the 8x8 element stiffness matrix for the displacement order
// [u1, v1, ..., u4, v4].
[[nodiscard]] math::DenseMatrix q4_plane_stress_stiffness_matrix(
    const Q4NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material,
    double thickness);
} // namespace finelemethod::elements
