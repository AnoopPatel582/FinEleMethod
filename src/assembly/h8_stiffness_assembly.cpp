#include "finelemethod/assembly/h8_stiffness_assembly.hpp"

#include "finelemethod/assembly/h8_dof_mapping.hpp"
#include "finelemethod/elements/h8_stiffness.hpp"

#include <cstddef>
#include <stdexcept>

namespace finelemethod::assembly
{
math::DenseMatrix assemble_h8_stiffness(const model::H8ElementCollection &element_collection,
                                        const model::NodeCollection &nodes,
                                        const model::MaterialCollection &materials,
                                        const model::DofMap &dof_map)
{
    if (dof_map.spatial_dimension() != model::SpatialDimension::three_dimensional)
    {
        throw std::invalid_argument("H8 stiffness assembly requires a three-dimensional DOF map.");
    }
    if (dof_map.size() == 0)
    {
        throw std::invalid_argument("Cannot assemble stiffness without degrees of freedom.");
    }

    math::DenseMatrix global_stiffness(dof_map.size(), dof_map.size());
    for (const model::H8Element &element : element_collection.elements())
    {
        const math::DenseMatrix element_stiffness =
            elements::h8_stiffness_matrix(element, nodes, materials);
        const H8GlobalDofIndices global_indices = h8_global_dof_indices(element, dof_map);

        for (std::size_t local_row = 0; local_row < global_indices.size(); ++local_row)
        {
            for (std::size_t local_column = 0; local_column < global_indices.size(); ++local_column)
            {
                global_stiffness(global_indices[local_row], global_indices[local_column]) +=
                    element_stiffness(local_row, local_column);
            }
        }
    }

    return global_stiffness;
}
} // namespace finelemethod::assembly
