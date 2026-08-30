#include "finelemethod/postprocessing/h8_results.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::math::DenseVector;
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::H8Element;
using finelemethod::model::H8ElementCollection;
using finelemethod::model::H8NodeIds;
using finelemethod::model::IsotropicElasticMaterial;
using finelemethod::model::MaterialCollection;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::SpatialDimension;
using finelemethod::postprocessing::recover_h8_model_results;

NodeCollection make_two_element_nodes()
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0, 0.0));
    nodes.add(Node(2, 1.0, 0.0, 0.0));
    nodes.add(Node(3, 2.0, 0.0, 0.0));
    nodes.add(Node(4, 0.0, 1.0, 0.0));
    nodes.add(Node(5, 1.0, 1.0, 0.0));
    nodes.add(Node(6, 2.0, 1.0, 0.0));
    nodes.add(Node(7, 0.0, 0.0, 1.0));
    nodes.add(Node(8, 1.0, 0.0, 1.0));
    nodes.add(Node(9, 2.0, 0.0, 1.0));
    nodes.add(Node(10, 0.0, 1.0, 1.0));
    nodes.add(Node(11, 1.0, 1.0, 1.0));
    nodes.add(Node(12, 2.0, 1.0, 1.0));
    return nodes;
}

TEST(H8ModelResults, RecoversEveryElementUsingGlobalDofsAndItsMaterial)
{
    const NodeCollection nodes = make_two_element_nodes();
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);
    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(1, 1000.0, 0.25));
    materials.add(IsotropicElasticMaterial(2, 2000.0, 0.25));
    H8ElementCollection elements;
    elements.add(H8Element(20, H8NodeIds{{1, 2, 5, 4, 7, 8, 11, 10}}, 1));
    elements.add(H8Element(10, H8NodeIds{{2, 3, 6, 5, 8, 9, 12, 11}}, 2));
    DenseVector global_displacements(dof_map.size());
    for (const Node &node : nodes.nodes())
    {
        global_displacements[dof_map.global_index(node.id(), DisplacementComponent::x)] =
            0.0025 * node.x();
        global_displacements[dof_map.global_index(node.id(), DisplacementComponent::y)] =
            0.0025 * node.y();
        global_displacements[dof_map.global_index(node.id(), DisplacementComponent::z)] =
            -0.01 * node.z();
    }

    const auto results =
        recover_h8_model_results(elements, nodes, materials, dof_map, global_displacements);

    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].element_id, 20);
    EXPECT_EQ(results[1].element_id, 10);
    constexpr double tolerance = 1.0e-12;
    for (const auto &point : results[0].gauss_points)
    {
        EXPECT_NEAR(point.strain[2], -0.01, tolerance);
        EXPECT_NEAR(point.stress[2], -10.0, tolerance);
    }
    for (const auto &point : results[1].gauss_points)
    {
        EXPECT_NEAR(point.strain[2], -0.01, tolerance);
        EXPECT_NEAR(point.stress[2], -20.0, tolerance);
    }
}

TEST(H8ModelResults, RejectsMismatchedGlobalDisplacementSize)
{
    const NodeCollection nodes = make_two_element_nodes();
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);

    EXPECT_THROW(static_cast<void>(recover_h8_model_results(
                     H8ElementCollection{}, nodes, MaterialCollection{}, dof_map, DenseVector(35))),
                 std::invalid_argument);
}

TEST(H8ModelResults, RejectsTwoDimensionalDofMap)
{
    const NodeCollection nodes = make_two_element_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);

    EXPECT_THROW(static_cast<void>(recover_h8_model_results(
                     H8ElementCollection{}, nodes, MaterialCollection{}, dof_map, DenseVector(24))),
                 std::invalid_argument);
}
} // namespace
