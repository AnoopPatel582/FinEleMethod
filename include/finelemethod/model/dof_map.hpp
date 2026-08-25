#pragma once

#include "finelemethod/model/node.hpp"
#include "finelemethod/model/node_collection.hpp"

#include <cstddef>
#include <unordered_map>
#include <vector>

namespace finelemethod::model
{
enum class SpatialDimension
{
    two_dimensional,
    three_dimensional
};

enum class DisplacementComponent
{
    x,
    y,
    z
};

class DofMap
{
  public:
    using size_type = std::size_t;

    DofMap(const NodeCollection &nodes, SpatialDimension spatial_dimension);

    [[nodiscard]] SpatialDimension spatial_dimension() const noexcept;
    [[nodiscard]] size_type degrees_of_freedom_per_node() const noexcept;
    [[nodiscard]] size_type size() const noexcept;

    [[nodiscard]] size_type global_index(NodeId node_id, DisplacementComponent component) const;
    [[nodiscard]] NodeId node_id(size_type global_index) const;
    [[nodiscard]] DisplacementComponent component(size_type global_index) const;

  private:
    [[nodiscard]] size_type component_offset(DisplacementComponent component) const;

    SpatialDimension spatial_dimension_;
    std::vector<NodeId> node_ids_;
    std::unordered_map<NodeId, size_type> node_indices_by_id_;
};
} // namespace finelemethod::model
