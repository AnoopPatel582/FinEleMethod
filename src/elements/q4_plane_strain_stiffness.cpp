#include "finelemethod/elements/q4_plane_strain_stiffness.hpp"

#include "finelemethod/elements/q4_gauss_quadrature.hpp"
#include "finelemethod/elements/q4_strain_displacement.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace finelemethod::elements
{
math::DenseMatrix q4_plane_strain_stiffness_matrix(const Q4NodeCoordinates &coordinates,
                                                   const model::IsotropicElasticMaterial &material,
                                                   const double thickness)
{
    if (!std::isfinite(thickness) || thickness <= 0.0)
    {
        throw std::invalid_argument("Q4 element thickness must be finite and greater than zero.");
    }

    const math::DenseMatrix constitutive = mechanics::plane_strain_constitutive_matrix(material);
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

math::DenseMatrix q4_plane_strain_stiffness_matrix(const model::Q4Element &element,
                                                   const model::NodeCollection &nodes,
                                                   const model::MaterialCollection &materials)
{
    Q4NodeCoordinates coordinates{};

    for (std::size_t node_index = 0; node_index < element.node_ids().size(); ++node_index)
    {
        const model::Node &node = nodes.at(element.node_ids()[node_index]);
        coordinates[node_index] = {node.x(), node.y()};
    }

    const model::IsotropicElasticMaterial &material = materials.at(element.material_id());
    return q4_plane_strain_stiffness_matrix(coordinates, material, element.thickness());
}
} // namespace finelemethod::elements
