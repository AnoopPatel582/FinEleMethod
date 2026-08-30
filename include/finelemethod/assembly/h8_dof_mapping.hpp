#pragma once

#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/h8_element.hpp"

#include <array>
#include <cstddef>

namespace finelemethod::assembly
{
using H8GlobalDofIndices = std::array<std::size_t, 24>;

// Returns global indices in the local order [u1, v1, w1, ..., u8, v8, w8].
[[nodiscard]] H8GlobalDofIndices h8_global_dof_indices(const model::H8Element &element,
                                                       const model::DofMap &dof_map);
} // namespace finelemethod::assembly
