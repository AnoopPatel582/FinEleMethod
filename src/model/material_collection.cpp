#include "finelemethod/model/material_collection.hpp"

#include <stdexcept>

namespace finelemethod::model
{
bool MaterialCollection::empty() const noexcept
{
    return materials_.empty();
}

MaterialCollection::size_type MaterialCollection::size() const noexcept
{
    return materials_.size();
}

bool MaterialCollection::contains(const MaterialId id) const noexcept
{
    return indices_by_id_.contains(id);
}

void MaterialCollection::add(IsotropicElasticMaterial material)
{
    const MaterialId id = material.id();
    if (contains(id))
    {
        throw std::invalid_argument("Material IDs must be unique within a model.");
    }

    const size_type index = materials_.size();
    materials_.push_back(material);

    try
    {
        indices_by_id_.emplace(id, index);
    }
    catch (...)
    {
        materials_.pop_back();
        throw;
    }
}

const IsotropicElasticMaterial &MaterialCollection::at(const MaterialId id) const
{
    const auto iterator = indices_by_id_.find(id);
    if (iterator == indices_by_id_.end())
    {
        throw std::out_of_range("Material ID was not found in the model.");
    }

    return materials_[iterator->second];
}

std::span<const IsotropicElasticMaterial> MaterialCollection::materials() const noexcept
{
    return materials_;
}
} // namespace finelemethod::model
