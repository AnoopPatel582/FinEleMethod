#include "finelemethod/input/abaqus_material_parser.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_isotropic_materials;

TEST(AbaqusMaterialParser, ParsesNamedIsotropicElasticMaterials)
{
    constexpr std::string_view input = R"(*Material, name=Steel
*Density
7850.0
*Elastic
210.0e9, 0.3
*MATERIAL, NAME=Aluminum
** isotropic is explicit here
*ELASTIC, TYPE=ISOTROPIC
70.0E9, 0.33
)";

    const auto materials = parse_abaqus_isotropic_materials(input);

    ASSERT_EQ(materials.size(), 2U);
    EXPECT_EQ(materials[0].name, "Steel");
    EXPECT_DOUBLE_EQ(materials[0].youngs_modulus, 210.0e9);
    EXPECT_DOUBLE_EQ(materials[0].poisson_ratio, 0.3);
    EXPECT_EQ(materials[1].name, "Aluminum");
    EXPECT_DOUBLE_EQ(materials[1].youngs_modulus, 70.0e9);
    EXPECT_DOUBLE_EQ(materials[1].poisson_ratio, 0.33);
}

TEST(AbaqusMaterialParser, RejectsDuplicateNamesWithoutCaseSensitivity)
{
    constexpr std::string_view input = R"(*Material, name=Steel
*Elastic
1000.0, 0.25
*Material, name=steel
*Elastic
2000.0, 0.3
)";

    EXPECT_THROW(static_cast<void>(parse_abaqus_isotropic_materials(input)), AbaqusParseError);
}

TEST(AbaqusMaterialParser, RejectsMaterialWithoutElasticData)
{
    constexpr std::string_view input = "*Material, name=Steel\n*Density\n7850.0\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_isotropic_materials(input)), AbaqusParseError);
}

TEST(AbaqusMaterialParser, RejectsInvalidElasticProperties)
{
    constexpr std::string_view input = "*Material, name=Steel\n*Elastic\n-1000.0, 0.3\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_isotropic_materials(input)), AbaqusParseError);
}

TEST(AbaqusMaterialParser, RejectsUnsupportedElasticType)
{
    constexpr std::string_view input =
        "*Material, name=Composite\n*Elastic, type=ENGINEERING CONSTANTS\n";

    EXPECT_THROW(static_cast<void>(parse_abaqus_isotropic_materials(input)), AbaqusParseError);
}
} // namespace
