#include "finelemethod/input/abaqus_cload_parser.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_point_loads;
using finelemethod::model::DisplacementComponent;

TEST(AbaqusCloadParser, ParsesMultipleSectionsAndPreservesRepeatedLoads)
{
    constexpr std::string_view input = R"(*Cload
2, 1, 10.5
3, 2, -4.0
*CLOAD
2, 1, 1.5
)";

    const auto loads = parse_abaqus_point_loads(input);

    ASSERT_EQ(loads.size(), 3U);
    EXPECT_EQ(loads[0].node_id(), 2U);
    EXPECT_EQ(loads[0].component(), DisplacementComponent::x);
    EXPECT_DOUBLE_EQ(loads[0].magnitude(), 10.5);
    EXPECT_EQ(loads[1].node_id(), 3U);
    EXPECT_EQ(loads[1].component(), DisplacementComponent::y);
    EXPECT_DOUBLE_EQ(loads[1].magnitude(), -4.0);
    EXPECT_EQ(loads[2].node_id(), 2U);
    EXPECT_EQ(loads[2].component(), DisplacementComponent::x);
    EXPECT_DOUBLE_EQ(loads[2].magnitude(), 1.5);
}

TEST(AbaqusCloadParser, RejectsNodeSetNamesUntilNodeSetsAreSupported)
{
    constexpr std::string_view input = "*Cload\nloaded, 1, 10.0\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_point_loads(input)), AbaqusParseError);
}

TEST(AbaqusCloadParser, RejectsUnsupportedTwoDimensionalDof)
{
    constexpr std::string_view input = "*Cload\n1, 3, 10.0\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_point_loads(input)), AbaqusParseError);
}

TEST(AbaqusCloadParser, RejectsMalformedDataRow)
{
    constexpr std::string_view input = "*Cload\n1, 1\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_point_loads(input)), AbaqusParseError);
}

TEST(AbaqusCloadParser, RequiresPointLoadData)
{
    EXPECT_THROW(static_cast<void>(parse_abaqus_point_loads("*Node\n1, 0, 0\n")), AbaqusParseError);
    EXPECT_THROW(static_cast<void>(parse_abaqus_point_loads("*Cload\n*Step\n")), AbaqusParseError);
}
} // namespace
