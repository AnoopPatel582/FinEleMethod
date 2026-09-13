#include "finelemethod/elements/q4_reduced_stiffness.hpp"

#include "finelemethod/elements/q4_plane_stress_stiffness.hpp"
#include "finelemethod/elements/q4_strain_displacement.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::elements
{
math::DenseMatrix q4_reduced_plane_stress_stiffness_matrix(
    const Q4NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material,
    const double thickness, const double stabilization_fraction)
{
    if (!std::isfinite(thickness) || thickness <= 0.0)
    {
        throw std::invalid_argument(
            "Q4 reduced-integration thickness must be positive and finite.");
    }
    if (!std::isfinite(stabilization_fraction) || stabilization_fraction <= 0.0 ||
        stabilization_fraction > 1.0)
    {
        throw std::invalid_argument(
            "Q4 hourglass stabilization fraction must be in the interval (0, 1].");
    }

    const Q4StrainDisplacement center = q4_strain_displacement_matrix(coordinates, 0.0, 0.0);
    const math::DenseMatrix constitutive = mechanics::plane_stress_constitutive_matrix(material);
    const math::DenseMatrix reduced = transpose(center.matrix) * constitutive * center.matrix *
                                      (4.0 * center.jacobian_determinant * thickness);
    const math::DenseMatrix full =
        q4_plane_stress_stiffness_matrix(coordinates, material, thickness);
    return reduced + (full - reduced) * stabilization_fraction;
}
} // namespace finelemethod::elements
