#pragma once

#include "finelemethod/elements/q4_jacobian.hpp"
#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/q4_element.hpp"

namespace finelemethod::elements
{
// Returns the 8x8 element stiffness matrix for the displacement order
// [u1, v1, ..., u4, v4].
[[nodiscard]] math::DenseMatrix q4_plane_stress_stiffness_matrix(
    const Q4NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material,
    double thickness);

// Resolves the element's node and material IDs from the model collections.
[[nodiscard]] math::DenseMatrix q4_plane_stress_stiffness_matrix(
    const model::Q4Element &element, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials);
} // namespace finelemethod::elements
