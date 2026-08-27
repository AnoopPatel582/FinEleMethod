#include "finelemethod/model/q4_element.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::model
{
namespace
{
void validate_distinct_node_ids(const Q4NodeIds &node_ids)
{
    for (std::size_t first = 0; first < node_ids.size(); ++first)
    {
        for (std::size_t second = first + 1; second < node_ids.size(); ++second)
        {
            if (node_ids[first] == node_ids[second])
            {
                throw std::invalid_argument("Q4 element node IDs must be distinct.");
            }
        }
    }
}
} // namespace

Q4Element::Q4Element(const ElementId id, Q4NodeIds node_ids, const MaterialId material_id,
                     const double thickness)
    : id_(id), node_ids_(node_ids), material_id_(material_id), thickness_(thickness)
{
    validate_distinct_node_ids(node_ids_);

    if (!std::isfinite(thickness_) || thickness_ <= 0.0)
    {
        throw std::invalid_argument("Q4 element thickness must be finite and greater than zero.");
    }
}

ElementId Q4Element::id() const noexcept
{
    return id_;
}

const Q4NodeIds &Q4Element::node_ids() const noexcept
{
    return node_ids_;
}

MaterialId Q4Element::material_id() const noexcept
{
    return material_id_;
}

double Q4Element::thickness() const noexcept
{
    return thickness_;
}
} // namespace finelemethod::model
