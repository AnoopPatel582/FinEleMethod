#include "finelemethod/assembly/h8_dof_mapping.hpp"

#include <cstddef>
#include <stdexcept>

namespace finelemethod::assembly
{
H8GlobalDofIndices h8_global_dof_indices(const model::H8Element &element,
                                         const model::DofMap &dof_map)
{
    if (dof_map.spatial_dimension() != model::SpatialDimension::three_dimensional)
    {
        throw std::invalid_argument("H8 elements require a three-dimensional DOF map.");
    }

    H8GlobalDofIndices indices{};
    for (std::size_t node_index = 0; node_index < element.node_ids().size(); ++node_index)
    {
        const model::NodeId node_id = element.node_ids()[node_index];
        indices[3 * node_index] = dof_map.global_index(node_id, model::DisplacementComponent::x);
        indices[3 * node_index + 1] =
            dof_map.global_index(node_id, model::DisplacementComponent::y);
        indices[3 * node_index + 2] =
            dof_map.global_index(node_id, model::DisplacementComponent::z);
    }

    return indices;
}
} // namespace finelemethod::assembly
