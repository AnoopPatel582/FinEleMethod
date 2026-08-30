#pragma once

#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/q4_element_collection.hpp"

namespace finelemethod::assembly
{
// Assembles Q4 plane-stress element matrices into the dense global matrix.
[[nodiscard]] math::DenseMatrix assemble_q4_plane_stress_stiffness(
    const model::Q4ElementCollection &element_collection, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map);

// Assembles Q4 plane-strain element matrices into the dense global matrix.
[[nodiscard]] math::DenseMatrix assemble_q4_plane_strain_stiffness(
    const model::Q4ElementCollection &element_collection, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map);
} // namespace finelemethod::assembly
