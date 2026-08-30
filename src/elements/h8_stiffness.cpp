#include "finelemethod/elements/h8_stiffness.hpp"

#include "finelemethod/elements/h8_gauss_quadrature.hpp"
#include "finelemethod/elements/h8_strain_displacement.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"

#include <cstddef>

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

math::DenseMatrix h8_stiffness_matrix(const model::H8Element &element,
                                      const model::NodeCollection &nodes,
                                      const model::MaterialCollection &materials)
{
    H8NodeCoordinates coordinates{};

    for (std::size_t node_index = 0; node_index < element.node_ids().size(); ++node_index)
    {
        const model::Node &node = nodes.at(element.node_ids()[node_index]);
        coordinates[node_index] = {node.x(), node.y(), node.z()};
    }

    return h8_stiffness_matrix(coordinates, materials.at(element.material_id()));
}
} // namespace finelemethod::elements
