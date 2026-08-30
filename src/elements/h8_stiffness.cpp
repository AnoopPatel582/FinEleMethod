#include "finelemethod/elements/h8_stiffness.hpp"

#include "finelemethod/elements/h8_gauss_quadrature.hpp"
#include "finelemethod/elements/h8_strain_displacement.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"

namespace finelemethod::elements
{
math::DenseMatrix h8_stiffness_matrix(const H8NodeCoordinates &coordinates,
                                      const model::IsotropicElasticMaterial &material)
{
    const math::DenseMatrix constitutive = mechanics::solid_isotropic_constitutive_matrix(material);
    math::DenseMatrix stiffness(24, 24);

    for (const H8GaussPoint &point : h8_gauss_quadrature_2x2x2())
    {
        const H8StrainDisplacement strain_displacement =
            h8_strain_displacement_matrix(coordinates, point.xi, point.eta, point.zeta);
        const double integration_scale = strain_displacement.jacobian_determinant * point.weight;

        stiffness += transpose(strain_displacement.matrix) * constitutive *
                     strain_displacement.matrix * integration_scale;
    }

    return stiffness;
}
} // namespace finelemethod::elements
