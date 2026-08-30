#include "finelemethod/model/h8_element_collection.hpp"

#include <stdexcept>
#include <utility>

namespace finelemethod::model
{
bool H8ElementCollection::empty() const noexcept
{
    return elements_.empty();
}

H8ElementCollection::size_type H8ElementCollection::size() const noexcept
{
    return elements_.size();
}

bool H8ElementCollection::contains(const ElementId id) const noexcept
{
    return indices_by_id_.contains(id);
}

void H8ElementCollection::add(H8Element element)
{
    const ElementId id = element.id();
    if (contains(id))
    {
        throw std::invalid_argument("Element IDs must be unique within a model.");
    }

    const size_type index = elements_.size();
    elements_.push_back(std::move(element));

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

const H8Element &H8ElementCollection::at(const ElementId id) const
{
    const auto iterator = indices_by_id_.find(id);
    if (iterator == indices_by_id_.end())
    {
        throw std::out_of_range("Element ID was not found in the model.");
    }

    return elements_[iterator->second];
}

std::span<const H8Element> H8ElementCollection::elements() const noexcept
{
    return elements_;
}
} // namespace finelemethod::model
