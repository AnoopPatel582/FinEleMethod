#include "finelemethod/model/dof_map.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::SpatialDimension;

NodeCollection make_nodes()
{
    NodeCollection nodes;
    nodes.add(Node(10, 0.0, 0.0));
    nodes.add(Node(5, 1.0, 0.0));
    return nodes;
}

TEST(DofMap, MapsTwoDimensionalComponentsInNodeOrder)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);

    EXPECT_EQ(dof_map.spatial_dimension(), SpatialDimension::two_dimensional);
    EXPECT_EQ(dof_map.degrees_of_freedom_per_node(), 2);
    EXPECT_EQ(dof_map.size(), 4);
    EXPECT_EQ(dof_map.global_index(10, DisplacementComponent::x), 0);
    EXPECT_EQ(dof_map.global_index(10, DisplacementComponent::y), 1);
    EXPECT_EQ(dof_map.global_index(5, DisplacementComponent::x), 2);
    EXPECT_EQ(dof_map.global_index(5, DisplacementComponent::y), 3);
}

TEST(DofMap, MapsThreeDimensionalComponentsInNodeOrder)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);

    EXPECT_EQ(dof_map.degrees_of_freedom_per_node(), 3);
    EXPECT_EQ(dof_map.size(), 6);
    EXPECT_EQ(dof_map.global_index(10, DisplacementComponent::z), 2);
    EXPECT_EQ(dof_map.global_index(5, DisplacementComponent::x), 3);
    EXPECT_EQ(dof_map.global_index(5, DisplacementComponent::z), 5);
}

TEST(DofMap, ReversesGlobalIndices)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);

    EXPECT_EQ(dof_map.node_id(0), 10);
    EXPECT_EQ(dof_map.component(0), DisplacementComponent::x);
    EXPECT_EQ(dof_map.node_id(4), 5);
    EXPECT_EQ(dof_map.component(4), DisplacementComponent::y);
    EXPECT_EQ(dof_map.node_id(5), 5);
    EXPECT_EQ(dof_map.component(5), DisplacementComponent::z);
}

TEST(DofMap, RejectsZComponentInTwoDimensionalModel)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);

    EXPECT_THROW(static_cast<void>(dof_map.global_index(10, DisplacementComponent::z)),
                 std::invalid_argument);
}

TEST(DofMap, RejectsUnknownNodeId)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);

    EXPECT_THROW(static_cast<void>(dof_map.global_index(99, DisplacementComponent::x)),
                 std::out_of_range);
}

TEST(DofMap, RejectsOutOfRangeGlobalIndex)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);

    EXPECT_THROW(static_cast<void>(dof_map.node_id(4)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(dof_map.component(4)), std::out_of_range);
}
} // namespace
