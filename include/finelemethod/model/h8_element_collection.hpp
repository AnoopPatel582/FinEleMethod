#pragma once

#include "finelemethod/model/h8_element.hpp"

#include <cstddef>
#include <span>
#include <unordered_map>
#include <vector>

namespace finelemethod::model
{
class H8ElementCollection
{
  public:
    using size_type = std::size_t;

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] size_type size() const noexcept;
    [[nodiscard]] bool contains(ElementId id) const noexcept;

    void add(H8Element element);

    [[nodiscard]] const H8Element &at(ElementId id) const;
    [[nodiscard]] std::span<const H8Element> elements() const noexcept;

  private:
    std::vector<H8Element> elements_;
    std::unordered_map<ElementId, size_type> indices_by_id_;
};
} // namespace finelemethod::model
