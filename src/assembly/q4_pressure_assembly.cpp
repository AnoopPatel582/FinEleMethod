#include "finelemethod/assembly/q4_pressure_assembly.hpp"

#include "finelemethod/assembly/q4_dof_mapping.hpp"
#include "finelemethod/elements/q4_edge_pressure.hpp"

#include <cstddef>
#include <stdexcept>

namespace finelemethod::assembly
{
math::DenseVector assemble_q4_edge_pressure_loads(
    const model::Q4ElementCollection &element_collection, const model::NodeCollection &nodes,
    const model::DofMap &dof_map, const std::span<const model::Q4EdgePressureLoad> pressure_loads)
{
    if (dof_map.spatial_dimension() != model::SpatialDimension::two_dimensional)
    {
        throw std::invalid_argument("Q4 pressure assembly requires a two-dimensional DOF map.");
    }
    if (dof_map.size() == 0)
    {
        throw std::invalid_argument("Cannot assemble pressure loads without degrees of freedom.");
    }

    math::DenseVector global_load(dof_map.size());
    for (const model::Q4EdgePressureLoad &pressure_load : pressure_loads)
    {
        const model::Q4Element &element = element_collection.at(pressure_load.element_id());
        elements::Q4NodeCoordinates coordinates{};
        for (std::size_t node_index = 0; node_index < element.node_ids().size(); ++node_index)
        {
            const model::Node &node = nodes.at(element.node_ids()[node_index]);
            coordinates[node_index] = {node.x(), node.y()};
        }

        const math::DenseVector element_load = elements::q4_uniform_edge_pressure_load(
            coordinates, pressure_load.edge(), pressure_load.pressure(), element.thickness());
        const Q4GlobalDofIndices global_indices = q4_global_dof_indices(element, dof_map);
        for (std::size_t local_dof = 0; local_dof < global_indices.size(); ++local_dof)
        {
            global_load[global_indices[local_dof]] += element_load[local_dof];
        }
    }
    return global_load;
}
} // namespace finelemethod::assembly
