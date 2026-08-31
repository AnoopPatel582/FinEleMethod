#pragma once

#include "finelemethod/math/coo_matrix.hpp"
#include "finelemethod/math/dense_matrix.hpp"

#include <cstddef>
#include <span>

namespace finelemethod::assembly
{
// Appends a square element matrix to a square global COO matrix using the
// supplied local-to-global degree-of-freedom mapping. The target is unchanged
// if validation fails.
void add_element_matrix_to_coo(math::CooMatrix &global_matrix,
                               const math::DenseMatrix &element_matrix,
                               std::span<const std::size_t> global_dof_indices);
} // namespace finelemethod::assembly
