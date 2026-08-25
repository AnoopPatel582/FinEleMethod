#include "finelemethod/model/node.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::model
{
namespace
{
void validate_coordinates(const double x, const double y, const double z)
{
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
    {
        throw std::invalid_argument("Node coordinates must be finite.");
    }
}
} // namespace

Node::Node(const NodeId id, const double x, const double y, const double z)
    : id_(id), x_(x), y_(y), z_(z)
{
    validate_coordinates(x, y, z);
}

NodeId Node::id() const noexcept
{
    return id_;
}

double Node::x() const noexcept
{
    return x_;
}

double Node::y() const noexcept
{
    return y_;
}

double Node::z() const noexcept
{
    return z_;
}
} // namespace finelemethod::model
