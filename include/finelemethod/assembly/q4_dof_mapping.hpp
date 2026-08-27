#pragma once

#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/q4_element.hpp"

#include <array>
#include <cstddef>

namespace finelemethod::assembly
{
using Q4GlobalDofIndices = std::array<std::size_t, 8>;

// Returns global indices in the local order [u1, v1, ..., u4, v4].
[[nodiscard]] Q4GlobalDofIndices q4_global_dof_indices(const model::Q4Element &element,
                                                       const model::DofMap &dof_map);
} // namespace finelemethod::assembly
