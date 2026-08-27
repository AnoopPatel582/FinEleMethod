#include "finelemethod/elements/q4_plane_stress_stiffness.hpp"

#include "finelemethod/elements/q4_gauss_quadrature.hpp"
#include "finelemethod/elements/q4_strain_displacement.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::elements
{
math::DenseMatrix q4_plane_stress_stiffness_matrix(const Q4NodeCoordinates &coordinates,
                                                   const model::IsotropicElasticMaterial &material,
                                                   const double thickness)
{
    if (!std::isfinite(thickness) || thickness <= 0.0)
    {
        throw std::invalid_argument("Q4 element thickness must be finite and greater than zero.");
    }

    const math::DenseMatrix constitutive = mechanics::plane_stress_constitutive_matrix(material);
    math::DenseMatrix stiffness(8, 8);

    for (const Q4GaussPoint &point : q4_gauss_quadrature_2x2())
    {
        const Q4StrainDisplacement strain_displacement =
            q4_strain_displacement_matrix(coordinates, point.xi, point.eta);
        const double integration_scale =
            thickness * strain_displacement.jacobian_determinant * point.weight;

        stiffness += transpose(strain_displacement.matrix) * constitutive *
                     strain_displacement.matrix * integration_scale;
    }

    return stiffness;
}
} // namespace finelemethod::elements
