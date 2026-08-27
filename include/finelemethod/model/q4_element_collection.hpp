#pragma once

#include "finelemethod/model/q4_element.hpp"

#include <cstddef>
#include <span>
#include <unordered_map>
#include <vector>

namespace finelemethod::model
{
class Q4ElementCollection
{
  public:
    using size_type = std::size_t;

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] size_type size() const noexcept;
    [[nodiscard]] bool contains(ElementId id) const noexcept;

    void add(Q4Element element);

    [[nodiscard]] const Q4Element &at(ElementId id) const;
    [[nodiscard]] std::span<const Q4Element> elements() const noexcept;

  private:
    std::vector<Q4Element> elements_;
    std::unordered_map<ElementId, size_type> indices_by_id_;
};
} // namespace finelemethod::model
