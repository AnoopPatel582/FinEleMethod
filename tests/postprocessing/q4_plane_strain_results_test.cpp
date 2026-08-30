#include "finelemethod/postprocessing/q4_plane_strain_results.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::math::DenseVector;
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::IsotropicElasticMaterial;
using finelemethod::model::MaterialCollection;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::Q4Element;
using finelemethod::model::Q4ElementCollection;
using finelemethod::model::Q4NodeIds;
using finelemethod::model::SpatialDimension;
using finelemethod::postprocessing::recover_q4_plane_strain_model_results;

NodeCollection make_two_element_nodes()
{
    NodeCollection nodes;
    nodes.add(Node(5, 1.0, 1.0));
    nodes.add(Node(1, 0.0, 0.0));
    nodes.add(Node(3, 2.0, 0.0));
    nodes.add(Node(6, 2.0, 1.0));
    nodes.add(Node(2, 1.0, 0.0));
    nodes.add(Node(4, 0.0, 1.0));
    return nodes;
}

TEST(Q4PlaneStrainModelResults, RecoversEveryElementUsingGlobalDofsAndItsMaterial)
{
    const NodeCollection nodes = make_two_element_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);

    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(1, 1000.0, 0.25));
    materials.add(IsotropicElasticMaterial(2, 2000.0, 0.25));

    Q4ElementCollection elements;
    elements.add(Q4Element(20, Q4NodeIds{{1, 2, 5, 4}}, 1, 1.0));
    elements.add(Q4Element(10, Q4NodeIds{{2, 3, 6, 5}}, 2, 1.0));

    DenseVector global_displacements(dof_map.size());
    for (const Node &node : nodes.nodes())
    {
        global_displacements[dof_map.global_index(node.id(), DisplacementComponent::x)] =
            0.01 * node.x();
    }

    const auto results = recover_q4_plane_strain_model_results(elements, nodes, materials, dof_map,
                                                               global_displacements);

    ASSERT_EQ(results.size(), 2U);
    EXPECT_EQ(results[0].element_id, 20U);
    EXPECT_EQ(results[1].element_id, 10U);
    constexpr double tolerance = 1.0e-12;
    for (const auto &point : results[0].gauss_points)
    {
        EXPECT_NEAR(point.strain[0], 0.01, tolerance);
        EXPECT_NEAR(point.stress[0], 12.0, tolerance);
        EXPECT_NEAR(point.stress[2], 4.0, tolerance);
        EXPECT_NEAR(point.von_mises, 8.0, tolerance);
    }
    for (const auto &point : results[1].gauss_points)
    {
        EXPECT_NEAR(point.strain[0], 0.01, tolerance);
        EXPECT_NEAR(point.stress[0], 24.0, tolerance);
        EXPECT_NEAR(point.stress[2], 8.0, tolerance);
        EXPECT_NEAR(point.von_mises, 16.0, tolerance);
    }
}

TEST(Q4PlaneStrainModelResults, RejectsMismatchedGlobalDisplacementSize)
{
    const NodeCollection nodes = make_two_element_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);

    EXPECT_THROW(static_cast<void>(recover_q4_plane_strain_model_results(
                     Q4ElementCollection{}, nodes, MaterialCollection{}, dof_map, DenseVector(11))),
                 std::invalid_argument);
}

TEST(Q4PlaneStrainModelResults, RejectsThreeDimensionalDofMap)
{
    const NodeCollection nodes = make_two_element_nodes();
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);

    EXPECT_THROW(static_cast<void>(recover_q4_plane_strain_model_results(
                     Q4ElementCollection{}, nodes, MaterialCollection{}, dof_map, DenseVector(18))),
                 std::invalid_argument);
}
} // namespace
