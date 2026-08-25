#pragma once

#include "finelemethod/model/node.hpp"

#include <cstddef>
#include <span>
#include <unordered_map>
#include <vector>

namespace finelemethod::model
{
class NodeCollection
{
  public:
    using size_type = std::size_t;

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] size_type size() const noexcept;
    [[nodiscard]] bool contains(NodeId id) const noexcept;

    void add(Node node);

    [[nodiscard]] const Node &at(NodeId id) const;
    [[nodiscard]] std::span<const Node> nodes() const noexcept;

  private:
    std::vector<Node> nodes_;
    std::unordered_map<NodeId, size_type> indices_by_id_;
};
} // namespace finelemethod::model
