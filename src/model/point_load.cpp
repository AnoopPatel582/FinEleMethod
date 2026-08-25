#include "finelemethod/model/point_load.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::model
{
PointLoad::PointLoad(const NodeId node_id, const DisplacementComponent component,
                     const double magnitude)
    : node_id_(node_id), component_(component), magnitude_(magnitude)
{
    if (!std::isfinite(magnitude))
    {
        throw std::invalid_argument("Point-load magnitude must be finite.");
    }
}

NodeId PointLoad::node_id() const noexcept
{
    return node_id_;
}

DisplacementComponent PointLoad::component() const noexcept
{
    return component_;
}

double PointLoad::magnitude() const noexcept
{
    return magnitude_;
}
} // namespace finelemethod::model
