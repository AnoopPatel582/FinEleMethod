#pragma once

#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/point_load.hpp"

#include <span>

namespace finelemethod::assembly
{
[[nodiscard]] math::DenseVector assemble_point_load_vector(
    const model::DofMap &dof_map, std::span<const model::PointLoad> point_loads);
} // namespace finelemethod::assembly
