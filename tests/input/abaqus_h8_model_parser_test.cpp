#include "finelemethod/input/abaqus_h8_model_parser.hpp"

#include "finelemethod/input/abaqus_parse_error.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::input::parse_abaqus_h8_model;
using finelemethod::model::DisplacementComponent;
using finelemethod::model::H8Face;
using finelemethod::model::H8NodeIds;

constexpr std::string_view valid_model = R"(*Node
1, 0, 0, 0
2, 1, 0, 0
3, 1, 1, 0
4, 0, 1, 0
5, 0, 0, 1
6, 1, 0, 1
7, 1, 1, 1
8, 0, 1, 1
*Material, name=Steel
*Elastic
200000, 0.3
*Element, type=C3D8, elset=BLOCK
10, 1, 2, 3, 4, 5, 6, 7, 8
*Solid Section, elset=block, material=steel
*Nset, nset=FIXED
1, 4, 5, 8
*Nset, nset=LOADED
2, 3, 6, 7
*Boundary
fixed, 1, 3
*Cload
loaded, 3, -25
*Dload
block, P2, 5
10, P1, 2
)";

TEST(AbaqusH8ModelParser, ConnectsSupportedThreeDimensionalModelData)
{
    const auto model = parse_abaqus_h8_model(valid_model);

    EXPECT_EQ(model.nodes.size(), 8U);
    ASSERT_EQ(model.materials.size(), 1U);
    EXPECT_DOUBLE_EQ(model.materials.at(1).youngs_modulus(), 200000.0);
    ASSERT_EQ(model.elements.size(), 1U);
    EXPECT_EQ(model.elements.at(10).node_ids(), (H8NodeIds{{1, 2, 3, 4, 5, 6, 7, 8}}));
    EXPECT_EQ(model.elements.at(10).material_id(), 1U);
    EXPECT_EQ(model.node_sets.size(), 2U);
    EXPECT_EQ(model.prescribed_displacements.size(), 12U);
    ASSERT_EQ(model.point_loads.size(), 4U);
    EXPECT_EQ(model.point_loads[0].node_id(), 2U);
    EXPECT_EQ(model.point_loads[0].component(), DisplacementComponent::z);
    EXPECT_DOUBLE_EQ(model.point_loads[0].magnitude(), -25.0);
    ASSERT_EQ(model.pressure_loads.size(), 2U);
    EXPECT_EQ(model.pressure_loads[0].element_id(), 10U);
    EXPECT_EQ(model.pressure_loads[0].face(), H8Face::two);
    EXPECT_DOUBLE_EQ(model.pressure_loads[0].pressure(), 5.0);
    EXPECT_EQ(model.pressure_loads[1].face(), H8Face::one);
}

TEST(AbaqusH8ModelParser, RejectsElementWithoutSolidSection)
{
    constexpr std::string_view input = R"(*Node
1,0,0,0
2,1,0,0
3,1,1,0
4,0,1,0
5,0,0,1
6,1,0,1
7,1,1,1
8,0,1,1
*Material, name=Steel
*Elastic
200000,0.3
*Element, type=C3D8, elset=block
1,1,2,3,4,5,6,7,8
*Solid Section, elset=other, material=Steel
*Boundary
1,1,3
)";

    EXPECT_THROW(static_cast<void>(parse_abaqus_h8_model(input)), AbaqusParseError);
}

TEST(AbaqusH8ModelParser, RejectsSolidSectionWithUnknownMaterial)
{
    constexpr std::string_view input = R"(*Node
1,0,0,0
*Material, name=Steel
*Elastic
200000,0.3
*Element, type=C3D8, elset=block
1,1,2,3,4,5,6,7,8
*Solid Section, elset=block, material=Unknown
*Boundary
1,1,3
)";

    EXPECT_THROW(static_cast<void>(parse_abaqus_h8_model(input)), AbaqusParseError);
}
} // namespace
