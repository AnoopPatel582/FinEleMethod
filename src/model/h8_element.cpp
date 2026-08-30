#include "finelemethod/model/h8_element.hpp"

#include <cstddef>
#include <stdexcept>

namespace finelemethod::model
{
namespace
{
void validate_distinct_node_ids(const H8NodeIds &node_ids)
{
    for (std::size_t first = 0; first < node_ids.size(); ++first)
    {
        for (std::size_t second = first + 1; second < node_ids.size(); ++second)
        {
            if (node_ids[first] == node_ids[second])
            {
                throw std::invalid_argument("H8 element node IDs must be distinct.");
            }
        }
    }
}
} // namespace

H8Element::H8Element(const ElementId id, H8NodeIds node_ids, const MaterialId material_id)
    : id_(id), node_ids_(node_ids), material_id_(material_id)
{
    validate_distinct_node_ids(node_ids_);
}

ElementId H8Element::id() const noexcept
{
    return id_;
}

const H8NodeIds &H8Element::node_ids() const noexcept
{
    return node_ids_;
}

MaterialId H8Element::material_id() const noexcept
{
    return material_id_;
}
} // namespace finelemethod::model
