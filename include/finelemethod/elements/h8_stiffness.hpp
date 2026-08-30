#pragma once

#include "finelemethod/elements/h8_jacobian.hpp"
#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/model/h8_element.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node_collection.hpp"

namespace finelemethod::elements
{
// Returns the 24x24 element stiffness matrix for the displacement order
// [u1, v1, w1, ..., u8, v8, w8].
[[nodiscard]] math::DenseMatrix h8_stiffness_matrix(
    const H8NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material);

// Resolves the element's node and material IDs from the model collections.
[[nodiscard]] math::DenseMatrix h8_stiffness_matrix(const model::H8Element &element,
                                                    const model::NodeCollection &nodes,
                                                    const model::MaterialCollection &materials);
} // namespace finelemethod::elements
