#include "finelemethod/solver/abaqus_q4_analysis.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <variant>

namespace
{
using finelemethod::input::Q4AnalysisType;
using finelemethod::solver::analyze_abaqus_q4;
using finelemethod::solver::Q4PlaneStrainAnalysisResult;
using finelemethod::solver::Q4PlaneStressAnalysisResult;

constexpr std::string_view input_template = R"(*Node
1, 0.0, 0.0
2, 1.0, 0.0
3, 1.0, 1.0
4, 0.0, 1.0
*Material, name=TestMaterial
*Elastic
1000.0, 0.25
*Element, type=TYPE_PLACEHOLDER, elset=plate
1, 1, 2, 3, 4
*Solid Section, elset=plate, material=TestMaterial
1.0
*Boundary
1, 1, 2
4, 1
*Dload
plate, P2, -10.0
)";

std::string input_with_type(const std::string_view element_type)
{
    std::string input(input_template);
    input.replace(input.find("TYPE_PLACEHOLDER"), std::string_view("TYPE_PLACEHOLDER").size(),
                  element_type);
    return input;
}

TEST(AbaqusQ4Analysis, DispatchesCps4ToPlaneStress)
{
    const auto solution = analyze_abaqus_q4(input_with_type("CPS4"));

    EXPECT_EQ(solution.model.analysis_type, Q4AnalysisType::plane_stress);
    ASSERT_TRUE(std::holds_alternative<Q4PlaneStressAnalysisResult>(solution.result));
    const auto &result = std::get<Q4PlaneStressAnalysisResult>(solution.result);
    EXPECT_NEAR(result.displacements[2], 0.01, 1.0e-12);
}

TEST(AbaqusQ4Analysis, DispatchesCpe4ToPlaneStrain)
{
    const auto solution = analyze_abaqus_q4(input_with_type("CPE4"));

    EXPECT_EQ(solution.model.analysis_type, Q4AnalysisType::plane_strain);
    ASSERT_TRUE(std::holds_alternative<Q4PlaneStrainAnalysisResult>(solution.result));
    const auto &result = std::get<Q4PlaneStrainAnalysisResult>(solution.result);
    EXPECT_NEAR(result.displacements[2], 0.009375, 1.0e-12);
}
} // namespace
