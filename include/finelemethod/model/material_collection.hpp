#pragma once

#include "finelemethod/model/isotropic_elastic_material.hpp"

#include <cstddef>
#include <span>
#include <unordered_map>
#include <vector>

namespace finelemethod::model
{
class MaterialCollection
{
  public:
    using size_type = std::size_t;

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] size_type size() const noexcept;
    [[nodiscard]] bool contains(MaterialId id) const noexcept;

    void add(IsotropicElasticMaterial material);

    [[nodiscard]] const IsotropicElasticMaterial &at(MaterialId id) const;
    [[nodiscard]] std::span<const IsotropicElasticMaterial> materials() const noexcept;

  private:
    std::vector<IsotropicElasticMaterial> materials_;
    std::unordered_map<MaterialId, size_type> indices_by_id_;
};
} // namespace finelemethod::model
