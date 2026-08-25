#pragma once

#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/node.hpp"

namespace finelemethod::model
{
class PointLoad
{
  public:
    PointLoad(NodeId node_id, DisplacementComponent component, double magnitude);

    [[nodiscard]] NodeId node_id() const noexcept;
    [[nodiscard]] DisplacementComponent component() const noexcept;
    [[nodiscard]] double magnitude() const noexcept;

  private:
    NodeId node_id_;
    DisplacementComponent component_;
    double magnitude_;
};
} // namespace finelemethod::model
