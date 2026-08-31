#include "finelemethod/assembly/q4_stiffness_assembly.hpp"

#include "finelemethod/assembly/coo_assembly.hpp"
#include "finelemethod/assembly/q4_dof_mapping.hpp"
#include "finelemethod/elements/q4_plane_strain_stiffness.hpp"
#include "finelemethod/elements/q4_plane_stress_stiffness.hpp"

#include <cstddef>
#include <stdexcept>

namespace
{
void validate_q4_stiffness_assembly(const finelemethod::model::DofMap &dof_map)
{
    if (dof_map.spatial_dimension() != finelemethod::model::SpatialDimension::two_dimensional)
    {
        throw std::invalid_argument("Q4 stiffness assembly requires a two-dimensional DOF map.");
    }
    if (dof_map.size() == 0)
    {
        throw std::invalid_argument("Cannot assemble stiffness without degrees of freedom.");
    }
}

template <typename ElementStiffnessFunction>
finelemethod::math::DenseMatrix assemble_q4_stiffness(
    const finelemethod::model::Q4ElementCollection &element_collection,
    const finelemethod::model::DofMap &dof_map, ElementStiffnessFunction element_stiffness_function)
{
    validate_q4_stiffness_assembly(dof_map);

    finelemethod::math::DenseMatrix global_stiffness(dof_map.size(), dof_map.size());
    for (const auto &element : element_collection.elements())
    {
        const auto element_stiffness = element_stiffness_function(element);
        const auto global_indices = finelemethod::assembly::q4_global_dof_indices(element, dof_map);

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

template <typename ElementStiffnessFunction>
finelemethod::math::CooMatrix assemble_q4_stiffness_coo(
    const finelemethod::model::Q4ElementCollection &element_collection,
    const finelemethod::model::DofMap &dof_map, ElementStiffnessFunction element_stiffness_function)
{
    validate_q4_stiffness_assembly(dof_map);

    finelemethod::math::CooMatrix global_stiffness(dof_map.size(), dof_map.size());
    for (const auto &element : element_collection.elements())
    {
        const auto element_stiffness = element_stiffness_function(element);
        const auto global_indices = finelemethod::assembly::q4_global_dof_indices(element, dof_map);
        finelemethod::assembly::add_element_matrix_to_coo(global_stiffness, element_stiffness,
                                                          global_indices);
    }

    return global_stiffness;
}
} // namespace

namespace finelemethod::assembly
{
math::DenseMatrix assemble_q4_plane_stress_stiffness(
    const model::Q4ElementCollection &element_collection, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map)
{
    return assemble_q4_stiffness(element_collection, dof_map, [&](const model::Q4Element &element) {
        return elements::q4_plane_stress_stiffness_matrix(element, nodes, materials);
    });
}

math::DenseMatrix assemble_q4_plane_strain_stiffness(
    const model::Q4ElementCollection &element_collection, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map)
{
    return assemble_q4_stiffness(element_collection, dof_map, [&](const model::Q4Element &element) {
        return elements::q4_plane_strain_stiffness_matrix(element, nodes, materials);
    });
}

math::CooMatrix assemble_q4_plane_stress_stiffness_coo(
    const model::Q4ElementCollection &element_collection, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map)
{
    return assemble_q4_stiffness_coo(
        element_collection, dof_map, [&](const model::Q4Element &element) {
            return elements::q4_plane_stress_stiffness_matrix(element, nodes, materials);
        });
}

math::CooMatrix assemble_q4_plane_strain_stiffness_coo(
    const model::Q4ElementCollection &element_collection, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map)
{
    return assemble_q4_stiffness_coo(
        element_collection, dof_map, [&](const model::Q4Element &element) {
            return elements::q4_plane_strain_stiffness_matrix(element, nodes, materials);
        });
}
} // namespace finelemethod::assembly
