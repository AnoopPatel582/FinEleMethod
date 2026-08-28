#include "finelemethod/input/abaqus_solid_section_parser.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_solid_sections;

TEST(AbaqusSolidSectionParser, ParsesAssignmentsAndThicknesses)
{
    constexpr std::string_view input = R"(*Solid Section, elset=left, material=Steel
0.01
*SOLID SECTION, MATERIAL=Aluminum, ELSET=right
2.5e-2,
)";

    const auto sections = parse_abaqus_solid_sections(input);

    ASSERT_EQ(sections.size(), 2U);
    EXPECT_EQ(sections[0].element_set, "left");
    EXPECT_EQ(sections[0].material_name, "Steel");
    EXPECT_DOUBLE_EQ(sections[0].thickness, 0.01);
    EXPECT_EQ(sections[1].element_set, "right");
    EXPECT_EQ(sections[1].material_name, "Aluminum");
    EXPECT_DOUBLE_EQ(sections[1].thickness, 0.025);
}

TEST(AbaqusSolidSectionParser, RejectsDuplicateElementSetWithoutCaseSensitivity)
{
    constexpr std::string_view input = R"(*Solid Section, elset=plate, material=Steel
1.0
*Solid Section, elset=PLATE, material=Steel
1.0
)";

    EXPECT_THROW(static_cast<void>(parse_abaqus_solid_sections(input)), AbaqusParseError);
}

TEST(AbaqusSolidSectionParser, RejectsMissingThickness)
{
    constexpr std::string_view input = "*Solid Section, elset=plate, material=Steel\n*Boundary\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_solid_sections(input)), AbaqusParseError);
}

TEST(AbaqusSolidSectionParser, RejectsNonpositiveThickness)
{
    constexpr std::string_view input = "*Solid Section, elset=plate, material=Steel\n0.0\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_solid_sections(input)), AbaqusParseError);
}

TEST(AbaqusSolidSectionParser, RequiresElementSetAndMaterialParameters)
{
    constexpr std::string_view missing_element_set = "*Solid Section, material=Steel\n1.0\n";
    constexpr std::string_view missing_material = "*Solid Section, elset=plate\n1.0\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_solid_sections(missing_element_set)),
                 AbaqusParseError);
    EXPECT_THROW(static_cast<void>(parse_abaqus_solid_sections(missing_material)),
                 AbaqusParseError);
}
} // namespace
