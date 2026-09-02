#include "finelemethod/model/q4_element_collection.hpp"

#include <stdexcept>

namespace finelemethod::model
{
bool Q4ElementCollection::empty() const noexcept
{
    return elements_.empty();
}

Q4ElementCollection::size_type Q4ElementCollection::size() const noexcept
{
    return elements_.size();
}

bool Q4ElementCollection::contains(const ElementId id) const noexcept
{
    return indices_by_id_.contains(id);
}

void Q4ElementCollection::add(Q4Element element)
{
    const ElementId id = element.id();
    if (contains(id))
    {
        throw std::invalid_argument("Element IDs must be unique within a model.");
    }

    const size_type index = elements_.size();
    elements_.push_back(element);

    try
    {
        indices_by_id_.emplace(id, index);
    }
    catch (...)
    {
        elements_.pop_back();
        throw;
    }
}

const Q4Element &Q4ElementCollection::at(const ElementId id) const
{
    const auto iterator = indices_by_id_.find(id);
    if (iterator == indices_by_id_.end())
    {
        throw std::out_of_range("Element ID was not found in the model.");
    }

    return elements_[iterator->second];
}

std::span<const Q4Element> Q4ElementCollection::elements() const noexcept
{
    return elements_;
}
} // namespace finelemethod::model
