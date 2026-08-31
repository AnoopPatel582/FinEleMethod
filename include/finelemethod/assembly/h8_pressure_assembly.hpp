#pragma once

#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/h8_element_collection.hpp"
#include "finelemethod/model/h8_face_pressure_load.hpp"
#include "finelemethod/model/node_collection.hpp"

#include <span>

namespace finelemethod::assembly
{
[[nodiscard]] math::DenseVector assemble_h8_face_pressure_loads(
    const model::H8ElementCollection &element_collection, const model::NodeCollection &nodes,
    const model::DofMap &dof_map, std::span<const model::H8FacePressureLoad> pressure_loads);
} // namespace finelemethod::assembly
