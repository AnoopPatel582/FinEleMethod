#pragma once

#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/h8_element_collection.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node_collection.hpp"

namespace finelemethod::assembly
{
// Assembles H8 element matrices into the dense global stiffness matrix.
[[nodiscard]] math::DenseMatrix assemble_h8_stiffness(
    const model::H8ElementCollection &element_collection, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map);
} // namespace finelemethod::assembly
