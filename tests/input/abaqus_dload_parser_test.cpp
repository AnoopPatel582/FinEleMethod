#include "finelemethod/input/abaqus_dload_parser.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <variant>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_h8_face_pressures;
using finelemethod::input::parse_abaqus_q4_edge_pressures;
using finelemethod::model::ElementId;
using finelemethod::model::H8Face;
using finelemethod::model::Q4Edge;

TEST(AbaqusDloadParser, ParsesElementIdsSetsEdgesAndRepeatedLoads)
{
    constexpr std::string_view input = R"(*Dload
10, P1, 25.0
loaded, p2, -4.5
*DLOAD
10, P4, 2.0
)";

    const auto loads = parse_abaqus_q4_edge_pressures(input);

    ASSERT_EQ(loads.size(), 3U);
    EXPECT_EQ(std::get<ElementId>(loads[0].target), 10U);
    EXPECT_EQ(loads[0].edge, Q4Edge::one);
    EXPECT_DOUBLE_EQ(loads[0].pressure, 25.0);
    EXPECT_EQ(std::get<std::string>(loads[1].target), "loaded");
    EXPECT_EQ(loads[1].edge, Q4Edge::two);
    EXPECT_DOUBLE_EQ(loads[1].pressure, -4.5);
    EXPECT_EQ(std::get<ElementId>(loads[2].target), 10U);
    EXPECT_EQ(loads[2].edge, Q4Edge::four);
    EXPECT_DOUBLE_EQ(loads[2].pressure, 2.0);
}

TEST(AbaqusDloadParser, RejectsUnsupportedLoadTypes)
{
    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_edge_pressures("*Dload\n1, P5, 2.0\n")),
                 AbaqusParseError);
    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_edge_pressures("*Dload\n1, BX, 2.0\n")),
                 AbaqusParseError);
}

TEST(AbaqusDloadParser, RejectsKeywordParametersUntilLoadHistoryIsSupported)
{
    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_edge_pressures("*Dload, OP=NEW\n1, P1, 2.0\n")),
                 AbaqusParseError);
}

TEST(AbaqusDloadParser, RejectsMalformedData)
{
    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_edge_pressures("*Dload\n1, P1\n")),
                 AbaqusParseError);
    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_edge_pressures("*Dload\n, P1, 2.0\n")),
                 AbaqusParseError);
    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_edge_pressures("*Dload\n1, P1, nan\n")),
                 AbaqusParseError);
}

TEST(AbaqusDloadParser, RequiresPressureData)
{
    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_edge_pressures("*Node\n1, 0, 0\n")),
                 AbaqusParseError);
    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_edge_pressures("*Dload\n*Step\n")),
                 AbaqusParseError);
}

TEST(AbaqusDloadParser, ParsesH8ElementIdsSetsFacesAndRepeatedLoads)
{
    constexpr std::string_view input = R"(*Dload
10, P1, 25.0
loaded, p6, -4.5
*DLOAD
10, P4, 2.0
)";

    const auto loads = parse_abaqus_h8_face_pressures(input);

    ASSERT_EQ(loads.size(), 3U);
    EXPECT_EQ(std::get<ElementId>(loads[0].target), 10U);
    EXPECT_EQ(loads[0].face, H8Face::one);
    EXPECT_DOUBLE_EQ(loads[0].pressure, 25.0);
    EXPECT_EQ(std::get<std::string>(loads[1].target), "loaded");
    EXPECT_EQ(loads[1].face, H8Face::six);
    EXPECT_DOUBLE_EQ(loads[1].pressure, -4.5);
    EXPECT_EQ(loads[2].face, H8Face::four);
}

TEST(AbaqusDloadParser, H8RejectsUnsupportedLoadTypes)
{
    EXPECT_THROW(static_cast<void>(parse_abaqus_h8_face_pressures("*Dload\n1, P7, 2.0\n")),
                 AbaqusParseError);
    EXPECT_THROW(static_cast<void>(parse_abaqus_h8_face_pressures("*Dload\n1, BX, 2.0\n")),
                 AbaqusParseError);
}
} // namespace
