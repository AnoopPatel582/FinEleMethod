#pragma once

#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/q4_edge_pressure_load.hpp"
#include "finelemethod/model/q4_element_collection.hpp"

#include <span>

namespace finelemethod::assembly
{
[[nodiscard]] math::DenseVector assemble_q4_edge_pressure_loads(
    const model::Q4ElementCollection &element_collection, const model::NodeCollection &nodes,
    const model::DofMap &dof_map, std::span<const model::Q4EdgePressureLoad> pressure_loads);
} // namespace finelemethod::assembly
