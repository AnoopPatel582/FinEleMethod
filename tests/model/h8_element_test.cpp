#include "finelemethod/model/h8_element.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::model::H8Element;
using finelemethod::model::H8NodeIds;

TEST(H8Element, StoresIdentityConnectivityAndMaterial)
{
    const H8NodeIds node_ids{{10, 20, 30, 40, 50, 60, 70, 80}};
    const H8Element element(7, node_ids, 3);

    EXPECT_EQ(element.id(), 7);
    EXPECT_EQ(element.node_ids(), node_ids);
    EXPECT_EQ(element.material_id(), 3);
}

TEST(H8Element, PreservesConnectivityOrder)
{
    const H8NodeIds node_ids{{8, 3, 12, 5, 20, 14, 9, 2}};
    const H8Element element(1, node_ids, 4);

    for (std::size_t index = 0; index < node_ids.size(); ++index)
    {
        EXPECT_EQ(element.node_ids()[index], node_ids[index]);
    }
}

TEST(H8Element, RejectsRepeatedNodeIds)
{
    EXPECT_THROW(H8Element(1, H8NodeIds{{1, 2, 3, 4, 5, 6, 7, 7}}, 1), std::invalid_argument);
    EXPECT_THROW(H8Element(1, H8NodeIds{{9, 2, 3, 4, 5, 6, 7, 9}}, 1), std::invalid_argument);
}
} // namespace
