#pragma once

#include <cstddef>

namespace finelemethod::model
{
using MaterialId = std::size_t;

class IsotropicElasticMaterial
{
  public:
    IsotropicElasticMaterial(MaterialId id, double youngs_modulus, double poisson_ratio);

    [[nodiscard]] MaterialId id() const noexcept;
    [[nodiscard]] double youngs_modulus() const noexcept;
    [[nodiscard]] double poisson_ratio() const noexcept;

  private:
    MaterialId id_;
    double youngs_modulus_;
    double poisson_ratio_;
};
} // namespace finelemethod::model
