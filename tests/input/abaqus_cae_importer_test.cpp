#include "finelemethod/input/abaqus_cae_importer.hpp"

#include "finelemethod/input/abaqus_parse_error.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>

namespace finelemethod::input
{
namespace
{
constexpr std::string_view cae_cps4r_model = R"(*Heading
*Part, name=Plate
*Node
1, 0.0, 0.0
2, 1.0, 0.0
3, 1.0, 1.0
4, 0.0, 1.0
*Element, type=CPS4R
1, 1, 2, 3, 4
*Nset, nset=Set-1, generate
1, 4, 1
*Elset, elset=Set-1, generate
1, 1, 1
*Solid Section, elset=Set-1, material=Steel
,
*End Part
*Assembly, name=Assembly
*Instance, name=Plate-1, part=Plate
*End Instance
*Nset, nset=Set-1, instance=Plate-1
1, 2
*Elset, elset=LoadedEdge, instance=Plate-1
1
*Surface, type=ELEMENT, name=PressureSurface
LoadedEdge, S2
*End Assembly
*Material, name=Steel
*Elastic
200000.0, 0.3
*Step
*Boundary
Set-1, 2, 2
*Dsload
PressureSurface, P, -1.0
*End Step
)";

std::string flat_model(const std::string_view element_type, const std::size_t node_count)
{
    std::string text = "*Node\n";
    for (std::size_t node = 1; node <= node_count; ++node)
    {
        text += std::to_string(node) + ", " + std::to_string(node - 1) + ", 0.0, 0.0\n";
    }
    text += "*Element, type=" + std::string(element_type) + ", elset=Body\n1";
    for (std::size_t node = 1; node <= node_count; ++node)
    {
        text += ", " + std::to_string(node);
    }
    text += "\n*Solid Section, elset=Body, material=Steel\n"
            "*Material, name=Steel\n"
            "*Elastic\n"
            "1000.0, 0.25\n"
            "*Boundary\n"
            "1, 1, 1\n";
    return text;
}

TEST(AbaqusCaeImporter, ResolvesAssemblySetsSurfacePressureAndDefaultSectionData)
{
    const AbaqusImportedModel model = import_abaqus_cae_model(cae_cps4r_model);

    EXPECT_EQ(model.element_type, AbaqusElementType::cps4r);
    EXPECT_EQ(model.nodes.size(), 4);
    ASSERT_EQ(model.elements.size(), 1);
    EXPECT_EQ(model.elements.front().material_name, "Steel");
    EXPECT_DOUBLE_EQ(model.elements.front().section_thickness, 1.0);
    EXPECT_EQ(model.prescribed_displacements.size(), 2);
    ASSERT_EQ(model.pressure_loads.size(), 1);
    EXPECT_EQ(model.pressure_loads.front().element_id, 1);
    EXPECT_EQ(model.pressure_loads.front().face, 2);
    EXPECT_DOUBLE_EQ(model.pressure_loads.front().magnitude, -1.0);
}

TEST(AbaqusCaeImporter, RecognizesEveryNewElementType)
{
    EXPECT_EQ(import_abaqus_cae_model(flat_model("CPS3", 3)).element_type, AbaqusElementType::cps3);
    EXPECT_EQ(import_abaqus_cae_model(flat_model("C3D4", 4)).element_type, AbaqusElementType::c3d4);
    EXPECT_EQ(import_abaqus_cae_model(flat_model("C3D8R", 8)).element_type,
              AbaqusElementType::c3d8r);
}

TEST(AbaqusCaeImporter, RejectsTransformedInstanceUntilTransformSupportIsImplemented)
{
    std::string input(cae_cps4r_model);
    const std::string marker = "*Instance, name=Plate-1, part=Plate\n";
    input.insert(input.find(marker) + marker.size(), "1.0, 0.0, 0.0\n");

    EXPECT_THROW(static_cast<void>(import_abaqus_cae_model(input)), AbaqusParseError);
}
} // namespace
} // namespace finelemethod::input
