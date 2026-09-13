#include "finelemethod/elements/h8_reduced_stiffness.hpp"

#include "finelemethod/elements/h8_stiffness.hpp"
#include "finelemethod/elements/h8_strain_displacement.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::elements
{
math::DenseMatrix h8_reduced_stiffness_matrix(const H8NodeCoordinates &coordinates,
                                              const model::IsotropicElasticMaterial &material,
                                              const double stabilization_fraction)
{
    if (!std::isfinite(stabilization_fraction) || stabilization_fraction <= 0.0 ||
        stabilization_fraction > 1.0)
    {
        throw std::invalid_argument(
            "H8 hourglass stabilization fraction must be in the interval (0, 1].");
    }

    const H8StrainDisplacement center = h8_strain_displacement_matrix(coordinates, 0.0, 0.0, 0.0);
    const math::DenseMatrix constitutive = mechanics::solid_isotropic_constitutive_matrix(material);
    // One center point represents the parent-cube volume of eight.
    const math::DenseMatrix reduced = transpose(center.matrix) * constitutive * center.matrix *
                                      (8.0 * center.jacobian_determinant);
    const math::DenseMatrix full = h8_stiffness_matrix(coordinates, material);
    // Stabilize only the stiffness modes missed by one-point integration:
    // Ke = Ke_reduced + alpha * (Ke_full - Ke_reduced).
    return reduced + (full - reduced) * stabilization_fraction;
}
} // namespace finelemethod::elements
