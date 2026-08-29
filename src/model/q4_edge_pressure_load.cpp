#include "finelemethod/model/q4_edge_pressure_load.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::model
{
Q4EdgePressureLoad::Q4EdgePressureLoad(const ElementId element_id, const Q4Edge edge,
                                       const double pressure)
    : element_id_(element_id), edge_(edge), pressure_(pressure)
{
    if (static_cast<std::size_t>(edge) > static_cast<std::size_t>(Q4Edge::four))
    {
        throw std::invalid_argument("Q4 pressure-load edge must be between one and four.");
    }
    if (!std::isfinite(pressure))
    {
        throw std::invalid_argument("Q4 pressure-load magnitude must be finite.");
    }
}

ElementId Q4EdgePressureLoad::element_id() const noexcept
{
    return element_id_;
}

Q4Edge Q4EdgePressureLoad::edge() const noexcept
{
    return edge_;
}

double Q4EdgePressureLoad::pressure() const noexcept
{
    return pressure_;
}
} // namespace finelemethod::model
