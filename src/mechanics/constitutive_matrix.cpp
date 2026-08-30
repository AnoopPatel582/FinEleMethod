#include "finelemethod/mechanics/constitutive_matrix.hpp"

namespace finelemethod::mechanics
{
math::DenseMatrix plane_stress_constitutive_matrix(const model::IsotropicElasticMaterial &material)
{
    const double youngs_modulus = material.youngs_modulus();
    const double poisson_ratio = material.poisson_ratio();
    const double factor = youngs_modulus / (1.0 - poisson_ratio * poisson_ratio);

    math::DenseMatrix constitutive_matrix(3, 3);
    constitutive_matrix(0, 0) = factor;
    constitutive_matrix(0, 1) = factor * poisson_ratio;
    constitutive_matrix(1, 0) = factor * poisson_ratio;
    constitutive_matrix(1, 1) = factor;
    constitutive_matrix(2, 2) = factor * (1.0 - poisson_ratio) / 2.0;

    return constitutive_matrix;
}

math::DenseMatrix plane_strain_constitutive_matrix(const model::IsotropicElasticMaterial &material)
{
    const double youngs_modulus = material.youngs_modulus();
    const double poisson_ratio = material.poisson_ratio();
    const double factor = youngs_modulus / ((1.0 + poisson_ratio) * (1.0 - 2.0 * poisson_ratio));

    math::DenseMatrix constitutive_matrix(3, 3);
    constitutive_matrix(0, 0) = factor * (1.0 - poisson_ratio);
    constitutive_matrix(0, 1) = factor * poisson_ratio;
    constitutive_matrix(1, 0) = factor * poisson_ratio;
    constitutive_matrix(1, 1) = factor * (1.0 - poisson_ratio);
    constitutive_matrix(2, 2) = factor * (1.0 - 2.0 * poisson_ratio) / 2.0;

    return constitutive_matrix;
}
} // namespace finelemethod::mechanics
