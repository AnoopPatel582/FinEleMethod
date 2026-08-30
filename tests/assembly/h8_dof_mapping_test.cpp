#include "finelemethod/assembly/h8_dof_mapping.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::assembly::h8_global_dof_indices;
using finelemethod::assembly::H8GlobalDofIndices;
using finelemethod::model::DofMap;
using finelemethod::model::H8Element;
using finelemethod::model::H8NodeIds;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::SpatialDimension;

NodeCollection make_nodes()
{
    NodeCollection nodes;
    nodes.add(Node(70, 1.0, 1.0, 1.0));
    nodes.add(Node(10, 0.0, 0.0, 0.0));
    nodes.add(Node(50, 0.0, 0.0, 1.0));
    nodes.add(Node(30, 1.0, 1.0, 0.0));
    nodes.add(Node(80, 0.0, 1.0, 1.0));
    nodes.add(Node(20, 1.0, 0.0, 0.0));
    nodes.add(Node(60, 1.0, 0.0, 1.0));
    nodes.add(Node(40, 0.0, 1.0, 0.0));
    return nodes;
}

TEST(H8DofMapping, MapsLocalDofsUsingElementConnectivityAndGlobalNodeOrder)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);
    const H8Element element(1, H8NodeIds{{10, 20, 30, 40, 50, 60, 70, 80}}, 1);

    const auto indices = h8_global_dof_indices(element, dof_map);

    const H8GlobalDofIndices expected{
        {3, 4, 5, 15, 16, 17, 9, 10, 11, 21, 22, 23, 6, 7, 8, 18, 19, 20, 0, 1, 2, 12, 13, 14}};
    EXPECT_EQ(indices, expected);
}

TEST(H8DofMapping, RejectsElementReferencingUnknownNode)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);
    const H8Element element(1, H8NodeIds{{10, 20, 30, 40, 50, 60, 70, 99}}, 1);

    EXPECT_THROW(static_cast<void>(h8_global_dof_indices(element, dof_map)), std::out_of_range);
}

TEST(H8DofMapping, RejectsTwoDimensionalDofMap)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const H8Element element(1, H8NodeIds{{10, 20, 30, 40, 50, 60, 70, 80}}, 1);

    EXPECT_THROW(static_cast<void>(h8_global_dof_indices(element, dof_map)), std::invalid_argument);
}
} // namespace
