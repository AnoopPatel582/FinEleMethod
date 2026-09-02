#include "finelemethod/model/node_collection.hpp"

#include <stdexcept>

namespace finelemethod::model
{
bool NodeCollection::empty() const noexcept
{
    return nodes_.empty();
}

NodeCollection::size_type NodeCollection::size() const noexcept
{
    return nodes_.size();
}

bool NodeCollection::contains(const NodeId id) const noexcept
{
    return indices_by_id_.contains(id);
}

void NodeCollection::add(Node node)
{
    const NodeId id = node.id();
    if (contains(id))
    {
        throw std::invalid_argument("Node IDs must be unique within a model.");
    }

    const size_type index = nodes_.size();
    nodes_.push_back(node);

    try
    {
        indices_by_id_.emplace(id, index);
    }
    catch (...)
    {
        nodes_.pop_back();
        throw;
    }
}

const Node &NodeCollection::at(const NodeId id) const
{
    const auto iterator = indices_by_id_.find(id);
    if (iterator == indices_by_id_.end())
    {
        throw std::out_of_range("Node ID was not found in the model.");
    }

    return nodes_[iterator->second];
}

std::span<const Node> NodeCollection::nodes() const noexcept
{
    return nodes_;
}
} // namespace finelemethod::model
