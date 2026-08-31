#include "finelemethod/input/abaqus_boundary_parser.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <variant>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_nodal_displacements;
using finelemethod::model::DisplacementComponent;
using finelemethod::model::SpatialDimension;

TEST(AbaqusBoundaryParser, ParsesSingleDofsRangesAndValues)
{
    constexpr std::string_view input = R"(*Boundary
1, 1
2, 2, 2, -0.125
*BOUNDARY, TYPE=DISPLACEMENT
4, 1, 2, 0.0
)";

    const auto displacements = parse_abaqus_nodal_displacements(input);

    ASSERT_EQ(displacements.size(), 4U);
    EXPECT_EQ(std::get<finelemethod::model::NodeId>(displacements[0].target), 1U);
    EXPECT_EQ(displacements[0].component, DisplacementComponent::x);
    EXPECT_DOUBLE_EQ(displacements[0].value, 0.0);
    EXPECT_EQ(std::get<finelemethod::model::NodeId>(displacements[1].target), 2U);
    EXPECT_EQ(displacements[1].component, DisplacementComponent::y);
    EXPECT_DOUBLE_EQ(displacements[1].value, -0.125);
    EXPECT_EQ(std::get<finelemethod::model::NodeId>(displacements[2].target), 4U);
    EXPECT_EQ(displacements[2].component, DisplacementComponent::x);
    EXPECT_EQ(displacements[3].component, DisplacementComponent::y);
}

TEST(AbaqusBoundaryParser, ParsesNodeSetTarget)
{
    constexpr std::string_view input = "*Boundary\nfixed, 1, 2\n";

    const auto displacements = parse_abaqus_nodal_displacements(input);

    ASSERT_EQ(displacements.size(), 2U);
    EXPECT_EQ(std::get<std::string>(displacements[0].target), "fixed");
    EXPECT_EQ(displacements[0].component, DisplacementComponent::x);
    EXPECT_EQ(std::get<std::string>(displacements[1].target), "fixed");
    EXPECT_EQ(displacements[1].component, DisplacementComponent::y);
}

TEST(AbaqusBoundaryParser, RejectsUnsupportedTwoDimensionalDof)
{
    constexpr std::string_view input = "*Boundary\n1, 3, 3\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_nodal_displacements(input)), AbaqusParseError);
}

TEST(AbaqusBoundaryParser, ParsesThreeDimensionalDofRange)
{
    constexpr std::string_view input = "*Boundary\nfixed, 1, 3, 0.0\n";

    const auto displacements =
        parse_abaqus_nodal_displacements(input, SpatialDimension::three_dimensional);

    ASSERT_EQ(displacements.size(), 3U);
    EXPECT_EQ(displacements[0].component, DisplacementComponent::x);
    EXPECT_EQ(displacements[1].component, DisplacementComponent::y);
    EXPECT_EQ(displacements[2].component, DisplacementComponent::z);
}

TEST(AbaqusBoundaryParser, RejectsDuplicateThreeDimensionalNodeComponent)
{
    constexpr std::string_view input = "*Boundary\n1, 3\n1, 3, 3, 0.0\n";

    EXPECT_THROW(static_cast<void>(
                     parse_abaqus_nodal_displacements(input, SpatialDimension::three_dimensional)),
                 AbaqusParseError);
}

TEST(AbaqusBoundaryParser, RejectsReversedDofRange)
{
    constexpr std::string_view input = "*Boundary\n1, 2, 1\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_nodal_displacements(input)), AbaqusParseError);
}

TEST(AbaqusBoundaryParser, RejectsDuplicateNodeComponent)
{
    constexpr std::string_view input = "*Boundary\n1, 1\n1, 1, 1, 0.0\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_nodal_displacements(input)), AbaqusParseError);
}

TEST(AbaqusBoundaryParser, RequiresBoundaryData)
{
    EXPECT_THROW(static_cast<void>(parse_abaqus_nodal_displacements("*Node\n1, 0, 0\n")),
                 AbaqusParseError);
    EXPECT_THROW(static_cast<void>(parse_abaqus_nodal_displacements("*Boundary\n*Step\n")),
                 AbaqusParseError);
}
} // namespace
