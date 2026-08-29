#pragma once

#include "finelemethod/model/q4_element.hpp"

#include <cstddef>

namespace finelemethod::model
{
enum class Q4Edge : std::size_t
{
    one,
    two,
    three,
    four
};

class Q4EdgePressureLoad
{
  public:
    Q4EdgePressureLoad(ElementId element_id, Q4Edge edge, double pressure);

    [[nodiscard]] ElementId element_id() const noexcept;
    [[nodiscard]] Q4Edge edge() const noexcept;
    [[nodiscard]] double pressure() const noexcept;

  private:
    ElementId element_id_;
    Q4Edge edge_;
    double pressure_;
};
} // namespace finelemethod::model
