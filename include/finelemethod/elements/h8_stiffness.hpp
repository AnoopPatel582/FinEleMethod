#pragma once

#include "finelemethod/elements/h8_jacobian.hpp"
#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

namespace finelemethod::elements
{
// Returns the 24x24 element stiffness matrix for the displacement order
// [u1, v1, w1, ..., u8, v8, w8].
[[nodiscard]] math::DenseMatrix h8_stiffness_matrix(
    const H8NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material);
} // namespace finelemethod::elements
