#include "finelemethod/model/q4_element_collection.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::model::Q4Element;
using finelemethod::model::Q4ElementCollection;
using finelemethod::model::Q4NodeIds;

TEST(Q4ElementCollection, StartsEmpty)
{
    const Q4ElementCollection elements;

    EXPECT_TRUE(elements.empty());
    EXPECT_EQ(elements.size(), 0);
    EXPECT_TRUE(elements.elements().empty());
}

TEST(Q4ElementCollection, AddsElementsAndFindsThemById)
{
    Q4ElementCollection elements;
    elements.add(Q4Element(10, Q4NodeIds{{1, 2, 3, 4}}, 1, 0.1));
    elements.add(Q4Element(25, Q4NodeIds{{2, 5, 6, 3}}, 2, 0.2));

    EXPECT_FALSE(elements.empty());
    EXPECT_EQ(elements.size(), 2);
    EXPECT_TRUE(elements.contains(10));
    EXPECT_TRUE(elements.contains(25));
    EXPECT_FALSE(elements.contains(99));
    EXPECT_EQ(elements.at(10).material_id(), 1);
    EXPECT_DOUBLE_EQ(elements.at(25).thickness(), 0.2);
}

TEST(Q4ElementCollection, PreservesInsertionOrder)
{
    Q4ElementCollection elements;
    elements.add(Q4Element(30, Q4NodeIds{{1, 2, 3, 4}}, 1, 1.0));
    elements.add(Q4Element(10, Q4NodeIds{{2, 5, 6, 3}}, 1, 1.0));
    elements.add(Q4Element(20, Q4NodeIds{{5, 7, 8, 6}}, 1, 1.0));

    const auto ordered_elements = elements.elements();

    ASSERT_EQ(ordered_elements.size(), 3);
    EXPECT_EQ(ordered_elements[0].id(), 30);
    EXPECT_EQ(ordered_elements[1].id(), 10);
    EXPECT_EQ(ordered_elements[2].id(), 20);
}

TEST(Q4ElementCollection, RejectsDuplicateIdWithoutChangingCollection)
{
    Q4ElementCollection elements;
    elements.add(Q4Element(10, Q4NodeIds{{1, 2, 3, 4}}, 1, 0.1));

    EXPECT_THROW(elements.add(Q4Element(10, Q4NodeIds{{5, 6, 7, 8}}, 2, 0.2)),
                 std::invalid_argument);
    EXPECT_EQ(elements.size(), 1);
    EXPECT_EQ(elements.at(10).node_ids(), (Q4NodeIds{{1, 2, 3, 4}}));
    EXPECT_EQ(elements.at(10).material_id(), 1);
    EXPECT_DOUBLE_EQ(elements.at(10).thickness(), 0.1);
}

TEST(Q4ElementCollection, RejectsMissingIdLookup)
{
    const Q4ElementCollection elements;

    EXPECT_THROW(static_cast<void>(elements.at(123)), std::out_of_range);
}
} // namespace
