#include "finelemethod/input/abaqus_q4_model_parser.hpp"

#include "finelemethod/input/abaqus_parse_error.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_q4_model;
using finelemethod::model::Q4NodeIds;

constexpr std::string_view valid_model = R"(*Node
1, 0.0, 0.0
2, 1.0, 0.0
3, 1.0, 1.0
4, 0.0, 1.0
5, 2.0, 0.0
6, 2.0, 1.0
*Material, name=Steel
*Elastic
200000.0, 0.3
*Material, name=Aluminum
*Elastic
70000.0, 0.33
*Element, type=CPS4, elset=LEFT
10, 1, 2, 3, 4
*Element, type=CPS4, elset=right
20, 2, 5, 6, 3
*Solid Section, elset=left, material=steel
0.01
*Solid Section, elset=RIGHT, material=ALUMINUM
0.02
*Boundary
1, 1, 2
4, 1
6, 2, 2, -0.1
*Cload
5, 1, 10.0
6, 2, -4.0
5, 1, 2.5
)";

TEST(AbaqusQ4ModelParser, ConnectsAllSupportedQ4ModelData)
{
    const auto model = parse_abaqus_q4_model(valid_model);

    EXPECT_EQ(model.nodes.size(), 6U);
    ASSERT_EQ(model.materials.size(), 2U);
    EXPECT_DOUBLE_EQ(model.materials.at(1).youngs_modulus(), 200000.0);
    EXPECT_DOUBLE_EQ(model.materials.at(2).youngs_modulus(), 70000.0);

    ASSERT_EQ(model.elements.size(), 2U);
    EXPECT_EQ(model.elements.at(10).node_ids(), (Q4NodeIds{{1, 2, 3, 4}}));
    EXPECT_EQ(model.elements.at(10).material_id(), 1U);
    EXPECT_DOUBLE_EQ(model.elements.at(10).thickness(), 0.01);
    EXPECT_EQ(model.elements.at(20).material_id(), 2U);
    EXPECT_DOUBLE_EQ(model.elements.at(20).thickness(), 0.02);

    ASSERT_EQ(model.prescribed_displacements.size(), 4U);
    EXPECT_EQ(model.prescribed_displacements[0].degree_of_freedom, 0U);
    EXPECT_DOUBLE_EQ(model.prescribed_displacements[0].value, 0.0);
    EXPECT_EQ(model.prescribed_displacements[1].degree_of_freedom, 1U);
    EXPECT_EQ(model.prescribed_displacements[2].degree_of_freedom, 6U);
    EXPECT_EQ(model.prescribed_displacements[3].degree_of_freedom, 11U);
    EXPECT_DOUBLE_EQ(model.prescribed_displacements[3].value, -0.1);

    ASSERT_EQ(model.point_loads.size(), 3U);
    EXPECT_EQ(model.point_loads[0].node_id(), 5U);
    EXPECT_EQ(model.point_loads[0].component(), finelemethod::model::DisplacementComponent::x);
    EXPECT_DOUBLE_EQ(model.point_loads[0].magnitude(), 10.0);
    EXPECT_EQ(model.point_loads[1].node_id(), 6U);
    EXPECT_EQ(model.point_loads[1].component(), finelemethod::model::DisplacementComponent::y);
    EXPECT_DOUBLE_EQ(model.point_loads[1].magnitude(), -4.0);
    EXPECT_EQ(model.point_loads[2].node_id(), 5U);
    EXPECT_DOUBLE_EQ(model.point_loads[2].magnitude(), 2.5);
}

TEST(AbaqusQ4ModelParser, RejectsBoundaryThatReferencesUnknownNode)
{
    constexpr std::string_view input = R"(*Node
1, 0.0, 0.0
2, 1.0, 0.0
3, 1.0, 1.0
4, 0.0, 1.0
*Material, name=Steel
*Elastic
200000.0, 0.3
*Element, type=CPS4, elset=plate
1, 1, 2, 3, 4
*Solid Section, elset=plate, material=Steel
1.0
*Boundary
99, 1, 2
)";

    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_model(input)), AbaqusParseError);
}

TEST(AbaqusQ4ModelParser, RejectsPointLoadThatReferencesUnknownNode)
{
    constexpr std::string_view input = R"(*Node
1, 0.0, 0.0
2, 1.0, 0.0
3, 1.0, 1.0
4, 0.0, 1.0
*Material, name=Steel
*Elastic
200000.0, 0.3
*Element, type=CPS4, elset=plate
1, 1, 2, 3, 4
*Solid Section, elset=plate, material=Steel
1.0
*Boundary
1, 1, 2
*Cload
99, 1, 10.0
)";

    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_model(input)), AbaqusParseError);
}

TEST(AbaqusQ4ModelParser, RejectsElementThatReferencesUnknownNode)
{
    constexpr std::string_view input = R"(*Node
1, 0.0, 0.0
2, 1.0, 0.0
3, 1.0, 1.0
*Material, name=Steel
*Elastic
200000.0, 0.3
*Element, type=CPS4, elset=plate
1, 1, 2, 3, 4
*Solid Section, elset=plate, material=Steel
1.0
)";

    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_model(input)), AbaqusParseError);
}

TEST(AbaqusQ4ModelParser, RejectsElementWithoutSolidSection)
{
    constexpr std::string_view input = R"(*Node
1, 0.0, 0.0
2, 1.0, 0.0
3, 1.0, 1.0
4, 0.0, 1.0
*Material, name=Steel
*Elastic
200000.0, 0.3
*Element, type=CPS4, elset=plate
1, 1, 2, 3, 4
*Solid Section, elset=other, material=Steel
1.0
)";

    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_model(input)), AbaqusParseError);
}

TEST(AbaqusQ4ModelParser, RejectsSolidSectionWithUnknownMaterial)
{
    constexpr std::string_view input = R"(*Node
1, 0.0, 0.0
2, 1.0, 0.0
3, 1.0, 1.0
4, 0.0, 1.0
*Material, name=Steel
*Elastic
200000.0, 0.3
*Element, type=CPS4, elset=plate
1, 1, 2, 3, 4
*Solid Section, elset=plate, material=Aluminum
1.0
)";

    EXPECT_THROW(static_cast<void>(parse_abaqus_q4_model(input)), AbaqusParseError);
}
} // namespace
