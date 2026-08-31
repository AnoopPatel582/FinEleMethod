#include "finelemethod/solver/abaqus_h8_analysis.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace
{
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::SpatialDimension;
using finelemethod::solver::analyze_abaqus_h8;
using finelemethod::solver::solve_abaqus_h8;

constexpr std::string_view compression_model = R"(*Node
1, 0, 0, 0
2, 1, 0, 0
3, 1, 1, 0
4, 0, 1, 0
5, 0, 0, 1
6, 1, 0, 1
7, 1, 1, 1
8, 0, 1, 1
*Material, name=TestMaterial
*Elastic
1000, 0.25
*Element, type=C3D8, elset=block
1, 1, 2, 3, 4, 5, 6, 7, 8
*Solid Section, elset=block, material=TestMaterial
*Nset, nset=BOTTOM
1, 2, 3, 4
*Nset, nset=XZERO
1, 4, 5, 8
*Nset, nset=YZERO
1, 2, 5, 6
*Nset, nset=TOP
5, 6, 7, 8
*Boundary
bottom, 3
xzero, 1
yzero, 2
*Cload
top, 3, -1.25
*Dload
block, P2, 5.0
)";

TEST(AbaqusH8Analysis, SolvesUniformUniaxialCompressionFromInputText)
{
    const auto result = solve_abaqus_h8(compression_model);

    constexpr double tolerance = 1.0e-12;
    ASSERT_EQ(result.displacements.size(), 24U);
    EXPECT_NEAR(result.displacements[0], 0.0, tolerance);
    EXPECT_NEAR(result.displacements[3], 0.0025, tolerance);
    EXPECT_NEAR(result.displacements[14], -0.01, tolerance);
    EXPECT_NEAR(result.displacements[21], 0.0, tolerance);
    EXPECT_NEAR(result.displacements[22], 0.0025, tolerance);
    EXPECT_NEAR(result.displacements[23], -0.01, tolerance);

    double bottom_reaction = 0.0;
    for (const std::size_t index : {2U, 5U, 8U, 11U})
    {
        bottom_reaction += result.reactions[index];
    }
    EXPECT_NEAR(bottom_reaction, 10.0, tolerance);

    ASSERT_EQ(result.element_results.size(), 1U);
    for (const auto &point : result.element_results[0].gauss_points)
    {
        EXPECT_NEAR(point.strain[0], 0.0025, tolerance);
        EXPECT_NEAR(point.strain[1], 0.0025, tolerance);
        EXPECT_NEAR(point.strain[2], -0.01, tolerance);
        EXPECT_NEAR(point.stress[0], 0.0, tolerance);
        EXPECT_NEAR(point.stress[1], 0.0, tolerance);
        EXPECT_NEAR(point.stress[2], -10.0, tolerance);
        EXPECT_NEAR(point.von_mises, 10.0, tolerance);
    }
}

TEST(AbaqusH8Analysis, RetainsModelAlongsideSolution)
{
    const auto solution = analyze_abaqus_h8(compression_model);
    const DofMap dof_map(solution.model.nodes, SpatialDimension::three_dimensional);

    EXPECT_EQ(solution.model.elements.size(), 1U);
    EXPECT_EQ(solution.model.materials.size(), 1U);
    EXPECT_NEAR(solution.result.displacements[dof_map.global_index(7, DisplacementComponent::z)],
                -0.01, 1.0e-12);
}
} // namespace
