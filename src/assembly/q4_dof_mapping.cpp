#include "finelemethod/assembly/q4_dof_mapping.hpp"

#include <stdexcept>

namespace finelemethod::assembly
{
Q4GlobalDofIndices q4_global_dof_indices(const model::Q4Element &element,
                                         const model::DofMap &dof_map)
{
    if (dof_map.spatial_dimension() != model::SpatialDimension::two_dimensional)
    {
        throw std::invalid_argument("Q4 elements require a two-dimensional DOF map.");
    }

    Q4GlobalDofIndices indices{};
    for (std::size_t node_index = 0; node_index < element.node_ids().size(); ++node_index)
    {
        const model::NodeId node_id = element.node_ids()[node_index];
        indices[2 * node_index] = dof_map.global_index(node_id, model::DisplacementComponent::x);
        indices[2 * node_index + 1] =
            dof_map.global_index(node_id, model::DisplacementComponent::y);
    }

    return indices;
}
} // namespace finelemethod::assembly
