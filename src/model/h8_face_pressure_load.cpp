#include "finelemethod/model/h8_face_pressure_load.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::model
{
H8FacePressureLoad::H8FacePressureLoad(const ElementId element_id, const H8Face face,
                                       const double pressure)
    : element_id_(element_id), face_(face), pressure_(pressure)
{
    if (static_cast<std::size_t>(face) > static_cast<std::size_t>(H8Face::six))
    {
        throw std::invalid_argument("H8 pressure-load face must be between one and six.");
    }
    if (!std::isfinite(pressure))
    {
        throw std::invalid_argument("H8 pressure-load magnitude must be finite.");
    }
}

ElementId H8FacePressureLoad::element_id() const noexcept
{
    return element_id_;
}

H8Face H8FacePressureLoad::face() const noexcept
{
    return face_;
}

double H8FacePressureLoad::pressure() const noexcept
{
    return pressure_;
}
} // namespace finelemethod::model
