#include "finelemethod/solver/abaqus_q4_plane_strain_analysis.hpp"

#include "finelemethod/input/abaqus_parse_error.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>

namespace
{
using finelemethod::input::AbaqusParseError;
using finelemethod::solver::solve_abaqus_q4_plane_strain;

constexpr std::string_view cpe4_input = R"(*Node
1, 0.0, 0.0
2, 1.0, 0.0
3, 1.0, 1.0
4, 0.0, 1.0
*Material, name=TestMaterial
*Elastic
1000.0, 0.25
*Element, type=CPE4, elset=plate
1, 1, 2, 3, 4
*Solid Section, elset=plate, material=TestMaterial
1.0
*Boundary
1, 1, 2
4, 1
*Dload
plate, P2, -10.0
)";

TEST(AbaqusQ4PlaneStrainAnalysis, SolvesUniformUniaxialLoadingFromInputText)
{
    const auto result = solve_abaqus_q4_plane_strain(cpe4_input);

    constexpr double tolerance = 1.0e-12;
    ASSERT_EQ(result.displacements.size(), 8U);
    EXPECT_NEAR(result.displacements[2], 0.009375, tolerance);
    EXPECT_NEAR(result.displacements[4], 0.009375, tolerance);
    EXPECT_NEAR(result.displacements[5], -0.003125, tolerance);
    EXPECT_NEAR(result.displacements[7], -0.003125, tolerance);
    ASSERT_EQ(result.element_results.size(), 1U);
    for (const auto &point : result.element_results[0].gauss_points)
    {
        EXPECT_NEAR(point.stress[0], 10.0, tolerance);
        EXPECT_NEAR(point.stress[1], 0.0, tolerance);
        EXPECT_NEAR(point.stress[2], 2.5, tolerance);
    }
}

TEST(AbaqusQ4PlaneStrainAnalysis, RejectsCps4PlaneStressModel)
{
    std::string input(cpe4_input);
    input.replace(input.find("CPE4"), 4, "CPS4");

    EXPECT_THROW(static_cast<void>(solve_abaqus_q4_plane_strain(input)), AbaqusParseError);
}
} // namespace
