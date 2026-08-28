#include "finelemethod/input/abaqus_node_parser.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_nodes;

TEST(AbaqusNodeParser, ParsesTwoAndThreeDimensionalCoordinates)
{
    constexpr std::string_view input = R"(** mesh
*Heading
Example model
*Node, nset=left
10, 0.0, 0.0
20, 1.0e+0, 0.0, 0.25
*ELEMENT, TYPE=CPS4
1, 10, 20, 30, 40
*node
30, 1.0, 1.0
40, 0.0, 1.0
)";

    const auto nodes = parse_abaqus_nodes(input);

    ASSERT_EQ(nodes.size(), 4U);
    EXPECT_DOUBLE_EQ(nodes.at(10).x(), 0.0);
    EXPECT_DOUBLE_EQ(nodes.at(10).y(), 0.0);
    EXPECT_DOUBLE_EQ(nodes.at(10).z(), 0.0);
    EXPECT_DOUBLE_EQ(nodes.at(20).x(), 1.0);
    EXPECT_DOUBLE_EQ(nodes.at(20).z(), 0.25);
    EXPECT_EQ(nodes.nodes()[2].id(), 30U);
    EXPECT_EQ(nodes.nodes()[3].id(), 40U);
}

TEST(AbaqusNodeParser, RejectsMalformedNodeRowWithLineNumber)
{
    constexpr std::string_view input = "*Node\n1, 0.0\n";

    try
    {
        static_cast<void>(parse_abaqus_nodes(input));
        FAIL() << "Expected AbaqusParseError.";
    }
    catch (const AbaqusParseError &exception)
    {
        EXPECT_NE(std::string_view(exception.what()).find("line 2"), std::string_view::npos);
    }
}

TEST(AbaqusNodeParser, RejectsDuplicateNodeId)
{
    constexpr std::string_view input = "*Node\n1, 0.0, 0.0\n1, 1.0, 0.0\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_nodes(input)), AbaqusParseError);
}

TEST(AbaqusNodeParser, RejectsInputWithoutNodeSection)
{
    constexpr std::string_view input = "*Heading\nNo mesh here\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_nodes(input)), AbaqusParseError);
}
} // namespace
