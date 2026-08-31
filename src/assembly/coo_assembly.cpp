#include "finelemethod/assembly/coo_assembly.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::assembly
{
void add_element_matrix_to_coo(math::CooMatrix &global_matrix,
                               const math::DenseMatrix &element_matrix,
                               const std::span<const std::size_t> global_dof_indices)
{
    if (global_matrix.rows() != global_matrix.columns())
    {
        throw std::invalid_argument("Global COO stiffness matrix must be square.");
    }
    if (element_matrix.rows() != element_matrix.columns() ||
        element_matrix.rows() != global_dof_indices.size())
    {
        throw std::invalid_argument("Element matrix dimensions must match its global DOF mapping.");
    }

    for (const std::size_t global_index : global_dof_indices)
    {
        if (global_index >= global_matrix.rows())
        {
            throw std::out_of_range("Element global DOF index is out of range.");
        }
    }
    for (std::size_t row = 0; row < element_matrix.rows(); ++row)
    {
        for (std::size_t column = 0; column < element_matrix.columns(); ++column)
        {
            if (!std::isfinite(element_matrix(row, column)))
            {
                throw std::invalid_argument("Element matrix values must be finite.");
            }
        }
    }

    for (std::size_t local_row = 0; local_row < global_dof_indices.size(); ++local_row)
    {
        for (std::size_t local_column = 0; local_column < global_dof_indices.size(); ++local_column)
        {
            global_matrix.add(global_dof_indices[local_row], global_dof_indices[local_column],
                              element_matrix(local_row, local_column));
        }
    }
}
} // namespace finelemethod::assembly
