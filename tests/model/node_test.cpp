#include "finelemethod/model/node.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::model::Node;

TEST(Node, StoresTwoDimensionalCoordinatesWithZeroZ)
{
    const Node node(42, 1.5, -2.0);

    EXPECT_EQ(node.id(), 42);
    EXPECT_DOUBLE_EQ(node.x(), 1.5);
    EXPECT_DOUBLE_EQ(node.y(), -2.0);
    EXPECT_DOUBLE_EQ(node.z(), 0.0);
}

TEST(Node, StoresThreeDimensionalCoordinates)
{
    const Node node(7, 1.0, 2.0, 3.0);

    EXPECT_EQ(node.id(), 7);
    EXPECT_DOUBLE_EQ(node.x(), 1.0);
    EXPECT_DOUBLE_EQ(node.y(), 2.0);
    EXPECT_DOUBLE_EQ(node.z(), 3.0);
}

TEST(Node, RejectsNonfiniteCoordinates)
{
    const double infinity = std::numeric_limits<double>::infinity();
    const double not_a_number = std::numeric_limits<double>::quiet_NaN();

    EXPECT_THROW(Node(1, infinity, 0.0, 0.0), std::invalid_argument);
    EXPECT_THROW(Node(1, 0.0, not_a_number, 0.0), std::invalid_argument);
    EXPECT_THROW(Node(1, 0.0, 0.0, -infinity), std::invalid_argument);
}
} // namespace
