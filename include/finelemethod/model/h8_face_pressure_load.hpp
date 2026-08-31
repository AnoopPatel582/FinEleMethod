#pragma once

#include "finelemethod/model/h8_element.hpp"

#include <cstddef>

namespace finelemethod::model
{
// Face numbering follows ABAQUS C3D8 surface labels S1 through S6.
enum class H8Face : std::size_t
{
    one,
    two,
    three,
    four,
    five,
    six
};

class H8FacePressureLoad
{
  public:
    H8FacePressureLoad(ElementId element_id, H8Face face, double pressure);

    [[nodiscard]] ElementId element_id() const noexcept;
    [[nodiscard]] H8Face face() const noexcept;
    [[nodiscard]] double pressure() const noexcept;

  private:
    ElementId element_id_;
    H8Face face_;
    double pressure_;
};
} // namespace finelemethod::model
