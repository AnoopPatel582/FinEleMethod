#pragma once

#include "finelemethod/model/isotropic_elastic_material.hpp"
#include "finelemethod/model/node.hpp"

#include <array>
#include <cstddef>

namespace finelemethod::model
{
using ElementId = std::size_t;
using Q4NodeIds = std::array<NodeId, 4>;

class Q4Element
{
  public:
    Q4Element(ElementId id, Q4NodeIds node_ids, MaterialId material_id, double thickness);

    [[nodiscard]] ElementId id() const noexcept;
    // Node IDs are stored in the Q4 counter-clockwise connectivity order.
    [[nodiscard]] const Q4NodeIds &node_ids() const noexcept;
    [[nodiscard]] MaterialId material_id() const noexcept;
    [[nodiscard]] double thickness() const noexcept;

  private:
    ElementId id_;
    Q4NodeIds node_ids_;
    MaterialId material_id_;
    double thickness_;
};
} // namespace finelemethod::model
