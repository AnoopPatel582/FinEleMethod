#include "finelemethod/postprocessing/h8_results.hpp"

#include "finelemethod/assembly/h8_dof_mapping.hpp"

#include <cstddef>
#include <stdexcept>

namespace finelemethod::postprocessing
{
std::vector<H8ElementResults> recover_h8_model_results(
    const model::H8ElementCollection &elements, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map,
    const math::DenseVector &global_displacements)
{
    if (dof_map.spatial_dimension() != model::SpatialDimension::three_dimensional)
    {
        throw std::invalid_argument("H8 result recovery requires a three-dimensional DOF map.");
    }
    if (global_displacements.size() != dof_map.size())
    {
        throw std::invalid_argument(
            "Global displacement vector size must match the number of degrees of freedom.");
    }

    std::vector<H8ElementResults> model_results;
    model_results.reserve(elements.size());

    for (const model::H8Element &element : elements.elements())
    {
        elements::H8NodeCoordinates coordinates{};
        for (std::size_t node_index = 0; node_index < element.node_ids().size(); ++node_index)
        {
            const model::Node &node = nodes.at(element.node_ids()[node_index]);
            coordinates[node_index] = {node.x(), node.y(), node.z()};
        }

        const assembly::H8GlobalDofIndices global_indices =
            assembly::h8_global_dof_indices(element, dof_map);
        math::DenseVector local_displacements(global_indices.size());
        for (std::size_t local_index = 0; local_index < global_indices.size(); ++local_index)
        {
            local_displacements[local_index] = global_displacements[global_indices[local_index]];
        }

        model_results.push_back(H8ElementResults{
            element.id(),
            elements::recover_h8_gauss_results(coordinates, materials.at(element.material_id()),
                                               local_displacements),
        });
    }

    return model_results;
}
} // namespace finelemethod::postprocessing
