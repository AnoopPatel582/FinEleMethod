#include "finelemethod/input/abaqus_cload_parser.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <variant>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_concentrated_loads;
using finelemethod::model::DisplacementComponent;

TEST(AbaqusCloadParser, ParsesMultipleSectionsAndPreservesRepeatedLoads)
{
    constexpr std::string_view input = R"(*Cload
2, 1, 10.5
3, 2, -4.0
*CLOAD
2, 1, 1.5
)";

    const auto loads = parse_abaqus_concentrated_loads(input);

    ASSERT_EQ(loads.size(), 3U);
    EXPECT_EQ(std::get<finelemethod::model::NodeId>(loads[0].target), 2U);
    EXPECT_EQ(loads[0].component, DisplacementComponent::x);
    EXPECT_DOUBLE_EQ(loads[0].magnitude, 10.5);
    EXPECT_EQ(std::get<finelemethod::model::NodeId>(loads[1].target), 3U);
    EXPECT_EQ(loads[1].component, DisplacementComponent::y);
    EXPECT_DOUBLE_EQ(loads[1].magnitude, -4.0);
    EXPECT_EQ(std::get<finelemethod::model::NodeId>(loads[2].target), 2U);
    EXPECT_EQ(loads[2].component, DisplacementComponent::x);
    EXPECT_DOUBLE_EQ(loads[2].magnitude, 1.5);
}

TEST(AbaqusCloadParser, ParsesNodeSetTarget)
{
    constexpr std::string_view input = "*Cload\nloaded, 1, 10.0\n";

    const auto loads = parse_abaqus_concentrated_loads(input);

    ASSERT_EQ(loads.size(), 1U);
    EXPECT_EQ(std::get<std::string>(loads[0].target), "loaded");
    EXPECT_EQ(loads[0].component, DisplacementComponent::x);
    EXPECT_DOUBLE_EQ(loads[0].magnitude, 10.0);
}

TEST(AbaqusCloadParser, RejectsUnsupportedTwoDimensionalDof)
{
    constexpr std::string_view input = "*Cload\n1, 3, 10.0\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_concentrated_loads(input)), AbaqusParseError);
}

TEST(AbaqusCloadParser, RejectsMalformedDataRow)
{
    constexpr std::string_view input = "*Cload\n1, 1\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_concentrated_loads(input)), AbaqusParseError);
}

TEST(AbaqusCloadParser, RequiresPointLoadData)
{
    EXPECT_THROW(static_cast<void>(parse_abaqus_concentrated_loads("*Node\n1, 0, 0\n")),
                 AbaqusParseError);
    EXPECT_THROW(static_cast<void>(parse_abaqus_concentrated_loads("*Cload\n*Step\n")),
                 AbaqusParseError);
}
} // namespace
