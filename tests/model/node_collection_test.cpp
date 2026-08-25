#include "finelemethod/model/node_collection.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;

TEST(NodeCollection, StartsEmpty)
{
    const NodeCollection nodes;

    EXPECT_TRUE(nodes.empty());
    EXPECT_EQ(nodes.size(), 0);
    EXPECT_TRUE(nodes.nodes().empty());
}

TEST(NodeCollection, AddsNodesAndFindsThemById)
{
    NodeCollection nodes;
    nodes.add(Node(10, 1.0, 2.0));
    nodes.add(Node(25, 3.0, 4.0, 5.0));

    EXPECT_FALSE(nodes.empty());
    EXPECT_EQ(nodes.size(), 2);
    EXPECT_TRUE(nodes.contains(10));
    EXPECT_TRUE(nodes.contains(25));
    EXPECT_FALSE(nodes.contains(99));
    EXPECT_DOUBLE_EQ(nodes.at(10).x(), 1.0);
    EXPECT_DOUBLE_EQ(nodes.at(25).z(), 5.0);
}

TEST(NodeCollection, PreservesInsertionOrder)
{
    NodeCollection nodes;
    nodes.add(Node(30, 0.0, 0.0));
    nodes.add(Node(10, 1.0, 0.0));
    nodes.add(Node(20, 2.0, 0.0));

    const auto ordered_nodes = nodes.nodes();

    ASSERT_EQ(ordered_nodes.size(), 3);
    EXPECT_EQ(ordered_nodes[0].id(), 30);
    EXPECT_EQ(ordered_nodes[1].id(), 10);
    EXPECT_EQ(ordered_nodes[2].id(), 20);
}

TEST(NodeCollection, RejectsDuplicateIdWithoutChangingCollection)
{
    NodeCollection nodes;
    nodes.add(Node(10, 1.0, 2.0));

    EXPECT_THROW(nodes.add(Node(10, 3.0, 4.0)), std::invalid_argument);
    EXPECT_EQ(nodes.size(), 1);
    EXPECT_DOUBLE_EQ(nodes.at(10).x(), 1.0);
    EXPECT_DOUBLE_EQ(nodes.at(10).y(), 2.0);
}

TEST(NodeCollection, RejectsMissingIdLookup)
{
    const NodeCollection nodes;

    EXPECT_THROW(static_cast<void>(nodes.at(123)), std::out_of_range);
}
} // namespace
