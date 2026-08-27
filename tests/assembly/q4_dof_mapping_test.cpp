#include "finelemethod/assembly/q4_dof_mapping.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::assembly::q4_global_dof_indices;
using finelemethod::assembly::Q4GlobalDofIndices;
using finelemethod::model::DofMap;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::Q4Element;
using finelemethod::model::Q4NodeIds;
using finelemethod::model::SpatialDimension;

NodeCollection make_nodes()
{
    NodeCollection nodes;
    nodes.add(Node(30, 1.0, 1.0));
    nodes.add(Node(10, 0.0, 0.0));
    nodes.add(Node(40, 0.0, 1.0));
    nodes.add(Node(20, 1.0, 0.0));
    return nodes;
}

TEST(Q4DofMapping, MapsLocalDofsUsingElementConnectivityAndGlobalNodeOrder)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const Q4Element element(1, Q4NodeIds{{10, 20, 30, 40}}, 1, 1.0);

    const auto indices = q4_global_dof_indices(element, dof_map);

    const Q4GlobalDofIndices expected{{2, 3, 6, 7, 0, 1, 4, 5}};
    EXPECT_EQ(indices, expected);
}

TEST(Q4DofMapping, RejectsElementReferencingUnknownNode)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const Q4Element element(1, Q4NodeIds{{10, 20, 30, 99}}, 1, 1.0);

    EXPECT_THROW(static_cast<void>(q4_global_dof_indices(element, dof_map)), std::out_of_range);
}

TEST(Q4DofMapping, RejectsThreeDimensionalDofMap)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);
    const Q4Element element(1, Q4NodeIds{{10, 20, 30, 40}}, 1, 1.0);

    EXPECT_THROW(static_cast<void>(q4_global_dof_indices(element, dof_map)), std::invalid_argument);
}
} // namespace
