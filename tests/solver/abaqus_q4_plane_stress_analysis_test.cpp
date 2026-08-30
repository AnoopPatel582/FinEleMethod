#include "finelemethod/solver/abaqus_q4_plane_stress_analysis.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace
{
using finelemethod::solver::solve_abaqus_q4_plane_stress;

TEST(AbaqusQ4PlaneStressAnalysis, SolvesUniformUniaxialTensionFromInputText)
{
    constexpr std::string_view input = R"(*Node
1, 0.0, 0.0
2, 1.0, 0.0
3, 1.0, 1.0
4, 0.0, 1.0
*Material, name=TestMaterial
*Elastic
1000.0, 0.25
*Element, type=CPS4, elset=plate
1, 1, 2, 3, 4
*Solid Section, elset=plate, material=TestMaterial
1.0
*Boundary
1, 1, 2
4, 1
*Dload
plate, P2, -10.0
)";

    const auto result = solve_abaqus_q4_plane_stress(input);

    constexpr double tolerance = 1.0e-12;
    ASSERT_EQ(result.displacements.size(), 8U);
    EXPECT_NEAR(result.displacements[0], 0.0, tolerance);
    EXPECT_NEAR(result.displacements[1], 0.0, tolerance);
    EXPECT_NEAR(result.displacements[2], 0.01, tolerance);
    EXPECT_NEAR(result.displacements[3], 0.0, tolerance);
    EXPECT_NEAR(result.displacements[4], 0.01, tolerance);
    EXPECT_NEAR(result.displacements[5], -0.0025, tolerance);
    EXPECT_NEAR(result.displacements[6], 0.0, tolerance);
    EXPECT_NEAR(result.displacements[7], -0.0025, tolerance);

    EXPECT_NEAR(result.reactions[0], -5.0, tolerance);
    EXPECT_NEAR(result.reactions[6], -5.0, tolerance);

    ASSERT_EQ(result.element_results.size(), 1U);
    for (const auto &point : result.element_results[0].gauss_points)
    {
        EXPECT_NEAR(point.strain[0], 0.01, tolerance);
        EXPECT_NEAR(point.strain[1], -0.0025, tolerance);
        EXPECT_NEAR(point.stress[0], 10.0, tolerance);
        EXPECT_NEAR(point.stress[1], 0.0, tolerance);
        EXPECT_NEAR(point.von_mises, 10.0, tolerance);
    }
}
} // namespace
