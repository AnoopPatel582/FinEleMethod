#include "finelemethod/input/abaqus_element_parser.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_h8_elements;
using finelemethod::input::parse_abaqus_q4_elements;
using finelemethod::input::Q4AnalysisType;
using finelemethod::model::H8NodeIds;
using finelemethod::model::Q4NodeIds;

TEST(AbaqusElementParser, ParsesCps4ConnectivityAndElementSets)
{
    constexpr std::string_view input = R"(*ELEMENT, TYPE=C3D8, ELSET=volume
50, 1, 2, 3, 4, 5, 6, 7, 8
*Element, type=CPS4, elset=left
20, 1, 2, 5, 4
** second Q4 section
*ELEMENT, ELSET=right, TYPE=CPS4
10, 2, 3, 6, 5
)";

    const auto elements = parse_abaqus_q4_elements(input);

    ASSERT_EQ(elements.size(), 2U);
    EXPECT_EQ(elements[0].id, 20U);
    EXPECT_EQ(elements[0].node_ids, (Q4NodeIds{{1, 2, 5, 4}}));
    EXPECT_EQ(elements[0].analysis_type, Q4AnalysisType::plane_stress);
    EXPECT_EQ(elements[0].element_set, "left");
    EXPECT_EQ(elements[1].id, 10U);
    EXPECT_EQ(elements[1].node_ids, (Q4NodeIds{{2, 3, 6, 5}}));
    EXPECT_EQ(elements[1].element_set, "right");
}

TEST(AbaqusElementParser, ParsesCpe4AsPlaneStrain)
{
    constexpr std::string_view input = "*Element, type=CPE4, elset=soil\n1, 10, 20, 30, 40\n";

    const auto elements = parse_abaqus_q4_elements(input);

    ASSERT_EQ(elements.size(), 1U);
    EXPECT_EQ(elements[0].analysis_type, Q4AnalysisType::plane_strain);
    EXPECT_EQ(elements[0].node_ids, (Q4NodeIds{{10, 20, 30, 40}}));
    EXPECT_EQ(elements[0].element_set, "soil");
}

TEST(AbaqusElementParser, RejectsMalformedCps4ConnectivityWithLineNumber)
{
    constexpr std::string_view input = "*Element, type=CPS4\n1, 10, 20, 30\n";

    try
    {
        static_cast<void>(parse_abaqus_q4_elements(input));
        FAIL() << "Expected AbaqusParseError.";
    }
    catch (const AbaqusParseError &exception)
    {
        EXPECT_NE(std::string_view(exception.what()).find("line 2"), std::string_view::npos);
    }
}

TEST(AbaqusElementParser, RejectsDuplicateQ4ElementId)
{
    constexpr std::string_view input = "*Element, type=CPS4\n1, 1, 2, 3, 4\n1, 5, 6, 7, 8\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_elements(input)), AbaqusParseError);
}

TEST(AbaqusElementParser, RejectsInputWithOnlyUnsupportedElementTypes)
{
    constexpr std::string_view input = "*Element, type=CPS4R\n1, 1, 2, 3, 4\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_elements(input)), AbaqusParseError);
}

TEST(AbaqusElementParser, ParsesC3d8ConnectivityAndElementSets)
{
    constexpr std::string_view input = R"(*Element, type=CPS4, elset=plate
1, 1, 2, 3, 4
*ELEMENT, ELSET=volume, TYPE=C3D8
50, 1, 2, 3, 4, 5, 6, 7, 8
)";

    const auto elements = parse_abaqus_h8_elements(input);

    ASSERT_EQ(elements.size(), 1U);
    EXPECT_EQ(elements[0].id, 50U);
    EXPECT_EQ(elements[0].node_ids, (H8NodeIds{{1, 2, 3, 4, 5, 6, 7, 8}}));
    EXPECT_EQ(elements[0].element_set, "volume");
}

TEST(AbaqusElementParser, RejectsMalformedC3d8ConnectivityWithLineNumber)
{
    constexpr std::string_view input = "*Element, type=C3D8\n1, 1, 2, 3, 4, 5, 6, 7\n";

    try
    {
        static_cast<void>(parse_abaqus_h8_elements(input));
        FAIL() << "Expected AbaqusParseError.";
    }
    catch (const AbaqusParseError &exception)
    {
        EXPECT_NE(std::string_view(exception.what()).find("line 2"), std::string_view::npos);
    }
}

TEST(AbaqusElementParser, RejectsDuplicateH8ElementId)
{
    constexpr std::string_view input = R"(*Element, type=C3D8
1, 1, 2, 3, 4, 5, 6, 7, 8
1, 8, 7, 6, 5, 4, 3, 2, 1
)";

    EXPECT_THROW(static_cast<void>(parse_abaqus_h8_elements(input)), AbaqusParseError);
}

TEST(AbaqusElementParser, H8RejectsInputWithOnlyUnsupportedElementTypes)
{
    constexpr std::string_view input = "*Element, type=C3D8R\n1, 1, 2, 3, 4, 5, 6, 7, 8\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_h8_elements(input)), AbaqusParseError);
}
} // namespace
