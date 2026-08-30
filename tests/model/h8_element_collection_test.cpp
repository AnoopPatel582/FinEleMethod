#include "finelemethod/model/h8_element_collection.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::model::H8Element;
using finelemethod::model::H8ElementCollection;
using finelemethod::model::H8NodeIds;

TEST(H8ElementCollection, StartsEmpty)
{
    const H8ElementCollection elements;

    EXPECT_TRUE(elements.empty());
    EXPECT_EQ(elements.size(), 0);
    EXPECT_TRUE(elements.elements().empty());
}

TEST(H8ElementCollection, AddsElementsAndFindsThemById)
{
    H8ElementCollection elements;
    elements.add(H8Element(10, H8NodeIds{{1, 2, 3, 4, 5, 6, 7, 8}}, 1));
    elements.add(H8Element(25, H8NodeIds{{5, 6, 7, 8, 9, 10, 11, 12}}, 2));

    EXPECT_FALSE(elements.empty());
    EXPECT_EQ(elements.size(), 2);
    EXPECT_TRUE(elements.contains(10));
    EXPECT_TRUE(elements.contains(25));
    EXPECT_FALSE(elements.contains(99));
    EXPECT_EQ(elements.at(10).material_id(), 1);
    EXPECT_EQ(elements.at(25).material_id(), 2);
}

TEST(H8ElementCollection, PreservesInsertionOrder)
{
    H8ElementCollection elements;
    elements.add(H8Element(30, H8NodeIds{{1, 2, 3, 4, 5, 6, 7, 8}}, 1));
    elements.add(H8Element(10, H8NodeIds{{5, 6, 7, 8, 9, 10, 11, 12}}, 1));
    elements.add(H8Element(20, H8NodeIds{{9, 10, 11, 12, 13, 14, 15, 16}}, 1));

    const auto ordered_elements = elements.elements();

    ASSERT_EQ(ordered_elements.size(), 3);
    EXPECT_EQ(ordered_elements[0].id(), 30);
    EXPECT_EQ(ordered_elements[1].id(), 10);
    EXPECT_EQ(ordered_elements[2].id(), 20);
}

TEST(H8ElementCollection, RejectsDuplicateIdWithoutChangingCollection)
{
    H8ElementCollection elements;
    const H8NodeIds original_node_ids{{1, 2, 3, 4, 5, 6, 7, 8}};
    elements.add(H8Element(10, original_node_ids, 1));

    EXPECT_THROW(elements.add(H8Element(10, H8NodeIds{{9, 10, 11, 12, 13, 14, 15, 16}}, 2)),
                 std::invalid_argument);
    EXPECT_EQ(elements.size(), 1);
    EXPECT_EQ(elements.at(10).node_ids(), original_node_ids);
    EXPECT_EQ(elements.at(10).material_id(), 1);
}

TEST(H8ElementCollection, RejectsMissingIdLookup)
{
    const H8ElementCollection elements;

    EXPECT_THROW(static_cast<void>(elements.at(123)), std::out_of_range);
}
} // namespace
