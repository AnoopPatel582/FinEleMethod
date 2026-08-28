#include "finelemethod/input/abaqus_boundary_parser.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_nodal_displacements;
using finelemethod::model::DisplacementComponent;

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
    EXPECT_EQ(displacements[0].node_id, 1U);
    EXPECT_EQ(displacements[0].component, DisplacementComponent::x);
    EXPECT_DOUBLE_EQ(displacements[0].value, 0.0);
    EXPECT_EQ(displacements[1].node_id, 2U);
    EXPECT_EQ(displacements[1].component, DisplacementComponent::y);
    EXPECT_DOUBLE_EQ(displacements[1].value, -0.125);
    EXPECT_EQ(displacements[2].node_id, 4U);
    EXPECT_EQ(displacements[2].component, DisplacementComponent::x);
    EXPECT_EQ(displacements[3].component, DisplacementComponent::y);
}

TEST(AbaqusBoundaryParser, RejectsNodeSetNamesUntilNodeSetsAreSupported)
{
    constexpr std::string_view input = "*Boundary\nfixed, 1, 2\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_nodal_displacements(input)), AbaqusParseError);
}

TEST(AbaqusBoundaryParser, RejectsUnsupportedTwoDimensionalDof)
{
    constexpr std::string_view input = "*Boundary\n1, 3, 3\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_nodal_displacements(input)), AbaqusParseError);
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
