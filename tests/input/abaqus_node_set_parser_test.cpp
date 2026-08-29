#include "finelemethod/input/abaqus_node_set_parser.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_node_sets;

TEST(AbaqusNodeSetParser, ParsesListsAndAppendsRepeatedDeclarations)
{
    constexpr std::string_view input = R"(*Nset, nset=Fixed
1, 4
7
*NSET, NSET=loaded
2, 3
*Nset, nset=FIXED
10, 12
)";

    const auto node_sets = parse_abaqus_node_sets(input);

    ASSERT_EQ(node_sets.size(), 2U);
    EXPECT_EQ(node_sets[0].name, "Fixed");
    EXPECT_EQ(node_sets[0].node_ids, (std::vector<finelemethod::model::NodeId>{1, 4, 7, 10, 12}));
    EXPECT_EQ(node_sets[1].name, "loaded");
    EXPECT_EQ(node_sets[1].node_ids, (std::vector<finelemethod::model::NodeId>{2, 3}));
}

TEST(AbaqusNodeSetParser, RejectsDuplicateNodeWithinSet)
{
    constexpr std::string_view input = "*Nset, nset=fixed\n1, 4\n*Nset, nset=FIXED\n4\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_node_sets(input)), AbaqusParseError);
}

TEST(AbaqusNodeSetParser, RequiresNodeSetName)
{
    constexpr std::string_view input = "*Nset\n1, 4\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_node_sets(input)), AbaqusParseError);
}

TEST(AbaqusNodeSetParser, RejectsGenerateUntilRangeExpansionIsSupported)
{
    constexpr std::string_view input = "*Nset, nset=fixed, generate\n1, 9, 2\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_node_sets(input)), AbaqusParseError);
}

TEST(AbaqusNodeSetParser, RequiresNodeSetData)
{
    EXPECT_THROW(static_cast<void>(parse_abaqus_node_sets("*Node\n1, 0, 0\n")), AbaqusParseError);
    EXPECT_THROW(static_cast<void>(parse_abaqus_node_sets("*Nset, nset=fixed\n*Step\n")),
                 AbaqusParseError);
}
} // namespace
