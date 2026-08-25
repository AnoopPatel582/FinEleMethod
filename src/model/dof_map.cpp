#include "finelemethod/model/dof_map.hpp"

#include <stdexcept>

namespace finelemethod::model
{
DofMap::DofMap(const NodeCollection &nodes, const SpatialDimension spatial_dimension)
    : spatial_dimension_(spatial_dimension)
{
    node_ids_.reserve(nodes.size());
    node_indices_by_id_.reserve(nodes.size());

    for (const Node &node : nodes.nodes())
    {
        const size_type node_index = node_ids_.size();
        node_ids_.push_back(node.id());
        node_indices_by_id_.emplace(node.id(), node_index);
    }
}

SpatialDimension DofMap::spatial_dimension() const noexcept
{
    return spatial_dimension_;
}

DofMap::size_type DofMap::degrees_of_freedom_per_node() const noexcept
{
    return spatial_dimension_ == SpatialDimension::two_dimensional ? 2 : 3;
}

DofMap::size_type DofMap::size() const noexcept
{
    return node_ids_.size() * degrees_of_freedom_per_node();
}

DofMap::size_type DofMap::global_index(const NodeId node_id,
                                       const DisplacementComponent component) const
{
    const auto node_iterator = node_indices_by_id_.find(node_id);
    if (node_iterator == node_indices_by_id_.end())
    {
        throw std::out_of_range("Cannot map a degree of freedom for an unknown node ID.");
    }

    return node_iterator->second * degrees_of_freedom_per_node() + component_offset(component);
}

NodeId DofMap::node_id(const size_type global_index) const
{
    if (global_index >= size())
    {
        throw std::out_of_range("Global degree-of-freedom index is out of range.");
    }

    return node_ids_[global_index / degrees_of_freedom_per_node()];
}

DisplacementComponent DofMap::component(const size_type global_index) const
{
    if (global_index >= size())
    {
        throw std::out_of_range("Global degree-of-freedom index is out of range.");
    }

    switch (global_index % degrees_of_freedom_per_node())
    {
    case 0:
        return DisplacementComponent::x;
    case 1:
        return DisplacementComponent::y;
    case 2:
        return DisplacementComponent::z;
    default:
        throw std::logic_error("Degree-of-freedom component offset is invalid.");
    }
}

DofMap::size_type DofMap::component_offset(const DisplacementComponent component) const
{
    switch (component)
    {
    case DisplacementComponent::x:
        return 0;
    case DisplacementComponent::y:
        return 1;
    case DisplacementComponent::z:
        if (spatial_dimension_ == SpatialDimension::three_dimensional)
        {
            return 2;
        }
        throw std::invalid_argument("Z displacement is unavailable in a two-dimensional model.");
    }

    throw std::invalid_argument("Displacement component is invalid.");
}
} // namespace finelemethod::model
