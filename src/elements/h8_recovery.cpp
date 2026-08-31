#include "finelemethod/elements/h8_recovery.hpp"

#include "finelemethod/elements/h8_gauss_quadrature.hpp"
#include "finelemethod/elements/h8_strain_displacement.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"
#include "finelemethod/mechanics/solid_stress_measures.hpp"

#include <cstddef>
#include <stdexcept>

namespace finelemethod::elements
{
H8GaussResults recover_h8_gauss_results(const H8NodeCoordinates &coordinates,
                                        const model::IsotropicElasticMaterial &material,
                                        const math::DenseVector &local_displacements)
{
    if (local_displacements.size() != 24)
    {
        throw std::invalid_argument("H8 result recovery requires 24 local displacements.");
    }

    const math::DenseMatrix constitutive = mechanics::solid_isotropic_constitutive_matrix(material);
    const H8GaussQuadrature quadrature = h8_gauss_quadrature_2x2x2();
    H8GaussResults results{};

    for (std::size_t point_index = 0; point_index < quadrature.size(); ++point_index)
    {
        const H8GaussPoint &point = quadrature[point_index];
        const H8StrainDisplacement strain_displacement =
            h8_strain_displacement_matrix(coordinates, point.xi, point.eta, point.zeta);
        const math::DenseVector strain_vector = strain_displacement.matrix * local_displacements;
        const math::DenseVector stress_vector = constitutive * strain_vector;
        const mechanics::SolidStressMeasures stress_measures =
            mechanics::calculate_solid_stress_measures(stress_vector[0], stress_vector[1],
                                                       stress_vector[2], stress_vector[3],
                                                       stress_vector[4], stress_vector[5]);

        results[point_index] = H8PointResult{
            point.xi,
            point.eta,
            point.zeta,
            {strain_vector[0], strain_vector[1], strain_vector[2], strain_vector[3],
             strain_vector[4], strain_vector[5]},
            {stress_vector[0], stress_vector[1], stress_vector[2], stress_vector[3],
             stress_vector[4], stress_vector[5]},
            stress_measures.von_mises,
            stress_measures.principal_stresses,
        };
    }

    return results;
}
} // namespace finelemethod::elements
