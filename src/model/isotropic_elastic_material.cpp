#include "finelemethod/model/isotropic_elastic_material.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::model
{
IsotropicElasticMaterial::IsotropicElasticMaterial(const MaterialId id, const double youngs_modulus,
                                                   const double poisson_ratio)
    : id_(id), youngs_modulus_(youngs_modulus), poisson_ratio_(poisson_ratio)
{
    if (!std::isfinite(youngs_modulus) || youngs_modulus <= 0.0)
    {
        throw std::invalid_argument("Young's modulus must be finite and positive.");
    }

    if (!std::isfinite(poisson_ratio) || poisson_ratio <= -1.0 || poisson_ratio >= 0.5)
    {
        throw std::invalid_argument(
            "Poisson's ratio must be finite and greater than -1 and less than 0.5.");
    }
}

MaterialId IsotropicElasticMaterial::id() const noexcept
{
    return id_;
}

double IsotropicElasticMaterial::youngs_modulus() const noexcept
{
    return youngs_modulus_;
}

double IsotropicElasticMaterial::poisson_ratio() const noexcept
{
    return poisson_ratio_;
}
} // namespace finelemethod::model
