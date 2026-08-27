#include "finelemethod/model/q4_element.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::model::Q4Element;
using finelemethod::model::Q4NodeIds;

TEST(Q4Element, StoresIdentityConnectivityMaterialAndThickness)
{
    const Q4NodeIds node_ids{{10, 20, 30, 40}};
    const Q4Element element(7, node_ids, 3, 0.25);

    EXPECT_EQ(element.id(), 7);
    EXPECT_EQ(element.node_ids(), node_ids);
    EXPECT_EQ(element.material_id(), 3);
    EXPECT_DOUBLE_EQ(element.thickness(), 0.25);
}

TEST(Q4Element, PreservesConnectivityOrder)
{
    const Q4Element element(1, Q4NodeIds{{4, 8, 2, 6}}, 1, 1.0);

    EXPECT_EQ(element.node_ids()[0], 4);
    EXPECT_EQ(element.node_ids()[1], 8);
    EXPECT_EQ(element.node_ids()[2], 2);
    EXPECT_EQ(element.node_ids()[3], 6);
}

TEST(Q4Element, RejectsRepeatedNodeIds)
{
    EXPECT_THROW(Q4Element(1, Q4NodeIds{{1, 2, 2, 4}}, 1, 1.0), std::invalid_argument);
    EXPECT_THROW(Q4Element(1, Q4NodeIds{{5, 2, 3, 5}}, 1, 1.0), std::invalid_argument);
}

TEST(Q4Element, RejectsNonpositiveOrNonfiniteThickness)
{
    const Q4NodeIds node_ids{{1, 2, 3, 4}};

    EXPECT_THROW(Q4Element(1, node_ids, 1, 0.0), std::invalid_argument);
    EXPECT_THROW(Q4Element(1, node_ids, 1, -0.1), std::invalid_argument);
    EXPECT_THROW(Q4Element(1, node_ids, 1, std::numeric_limits<double>::infinity()),
                 std::invalid_argument);
    EXPECT_THROW(Q4Element(1, node_ids, 1, std::numeric_limits<double>::quiet_NaN()),
                 std::invalid_argument);
}
} // namespace
