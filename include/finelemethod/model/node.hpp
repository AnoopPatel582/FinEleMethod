#pragma once

#include <cstddef>

namespace finelemethod::model
{
using NodeId = std::size_t;

class Node
{
  public:
    Node(NodeId id, double x, double y, double z = 0.0);

    [[nodiscard]] NodeId id() const noexcept;
    [[nodiscard]] double x() const noexcept;
    [[nodiscard]] double y() const noexcept;
    [[nodiscard]] double z() const noexcept;

  private:
    NodeId id_;
    double x_;
    double y_;
    double z_;
};
} // namespace finelemethod::model
