#include "finelemethod/elements/q4_plane_stress_recovery.hpp"

#include "finelemethod/elements/q4_gauss_quadrature.hpp"
#include "finelemethod/elements/q4_strain_displacement.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"
#include "finelemethod/mechanics/plane_stress_measures.hpp"

#include <cstddef>
#include <stdexcept>

namespace finelemethod::elements
{
Q4PlaneStressGaussResults recover_q4_plane_stress_gauss_results(
    const Q4NodeCoordinates &coordinates, const model::IsotropicElasticMaterial &material,
    const math::DenseVector &local_displacements)
{
    if (local_displacements.size() != 8)
    {
        throw std::invalid_argument("Q4 result recovery requires eight local displacements.");
    }

    const auto constitutive = mechanics::plane_stress_constitutive_matrix(material);
    const auto quadrature = q4_gauss_quadrature_2x2();
    Q4PlaneStressGaussResults results{};

    for (std::size_t point_index = 0; point_index < quadrature.size(); ++point_index)
    {
        const Q4GaussPoint &point = quadrature[point_index];
        const auto strain_displacement =
            q4_strain_displacement_matrix(coordinates, point.xi, point.eta);
        const math::DenseVector strain_vector = strain_displacement.matrix * local_displacements;
        const math::DenseVector stress_vector = constitutive * strain_vector;
        const auto stress_measures = mechanics::calculate_plane_stress_measures(
            stress_vector[0], stress_vector[1], stress_vector[2]);

        results[point_index] = Q4PlaneStressPointResult{
            point.xi,
            point.eta,
            {strain_vector[0], strain_vector[1], strain_vector[2]},
            {stress_vector[0], stress_vector[1], stress_vector[2]},
            stress_measures.von_mises,
            stress_measures.principal_stresses,
        };
    }

    return results;
}
} // namespace finelemethod::elements
