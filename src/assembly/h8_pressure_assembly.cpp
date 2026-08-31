#include "finelemethod/assembly/h8_pressure_assembly.hpp"

#include "finelemethod/assembly/h8_dof_mapping.hpp"
#include "finelemethod/elements/h8_face_pressure.hpp"

#include <cstddef>
#include <stdexcept>

namespace finelemethod::assembly
{
math::DenseVector assemble_h8_face_pressure_loads(
    const model::H8ElementCollection &element_collection, const model::NodeCollection &nodes,
    const model::DofMap &dof_map, const std::span<const model::H8FacePressureLoad> pressure_loads)
{
    if (dof_map.spatial_dimension() != model::SpatialDimension::three_dimensional)
    {
        throw std::invalid_argument("H8 pressure assembly requires a three-dimensional DOF map.");
    }
    if (dof_map.size() == 0)
    {
        throw std::invalid_argument("Cannot assemble pressure loads without degrees of freedom.");
    }

    math::DenseVector global_load(dof_map.size());
    for (const model::H8FacePressureLoad &pressure_load : pressure_loads)
    {
        const model::H8Element &element = element_collection.at(pressure_load.element_id());
        elements::H8NodeCoordinates coordinates{};
        for (std::size_t node_index = 0; node_index < element.node_ids().size(); ++node_index)
        {
            const model::Node &node = nodes.at(element.node_ids()[node_index]);
            coordinates[node_index] = {node.x(), node.y(), node.z()};
        }

        const math::DenseVector element_load = elements::h8_uniform_face_pressure_load(
            coordinates, pressure_load.face(), pressure_load.pressure());
        const H8GlobalDofIndices global_indices = h8_global_dof_indices(element, dof_map);
        for (std::size_t local_dof = 0; local_dof < global_indices.size(); ++local_dof)
        {
            global_load[global_indices[local_dof]] += element_load[local_dof];
        }
    }
    return global_load;
}
} // namespace finelemethod::assembly
