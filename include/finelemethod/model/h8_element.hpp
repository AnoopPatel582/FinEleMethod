#pragma once

#include "finelemethod/model/element_id.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"
#include "finelemethod/model/node.hpp"

#include <array>

namespace finelemethod::model
{
using H8NodeIds = std::array<NodeId, 8>;

class H8Element
{
  public:
    H8Element(ElementId id, H8NodeIds node_ids, MaterialId material_id);

    [[nodiscard]] ElementId id() const noexcept;
    // Node IDs follow the standard H8 natural connectivity order.
    [[nodiscard]] const H8NodeIds &node_ids() const noexcept;
    [[nodiscard]] MaterialId material_id() const noexcept;

  private:
    ElementId id_;
    H8NodeIds node_ids_;
    MaterialId material_id_;
};
} // namespace finelemethod::model
