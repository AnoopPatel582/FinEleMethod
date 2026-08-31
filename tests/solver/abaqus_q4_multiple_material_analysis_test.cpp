#include "finelemethod/input/abaqus_input_file.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/solver/abaqus_q4_plane_stress_analysis.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::SpatialDimension;

TEST(AbaqusQ4MultipleMaterialAnalysis, UsesEachElementsAssignedElasticMaterial)
{
    const std::filesystem::path input_path = std::filesystem::path(FINELEMETHOD_SOURCE_DIR) /
                                             "examples" / "abaqus" / "q4_multiple_materials.inp";
    const std::string input_text = finelemethod::input::read_abaqus_input_file(input_path);
    const auto solution = finelemethod::solver::analyze_abaqus_q4_plane_stress(input_text);
    const DofMap dof_map(solution.model.nodes, SpatialDimension::two_dimensional);

    ASSERT_EQ(solution.model.materials.size(), 2U);
    ASSERT_EQ(solution.model.elements.size(), 2U);
    EXPECT_NE(solution.model.elements.at(1).material_id(),
              solution.model.elements.at(2).material_id());

    constexpr double tolerance = 1.0e-11;
    EXPECT_NEAR(solution.result.displacements[dof_map.global_index(2, DisplacementComponent::x)],
                0.01, tolerance);
    EXPECT_NEAR(solution.result.displacements[dof_map.global_index(5, DisplacementComponent::x)],
                0.01, tolerance);
    EXPECT_NEAR(solution.result.displacements[dof_map.global_index(3, DisplacementComponent::x)],
                0.015, tolerance);
    EXPECT_NEAR(solution.result.displacements[dof_map.global_index(6, DisplacementComponent::x)],
                0.015, tolerance);

    double horizontal_reaction = 0.0;
    for (const std::size_t node_id : {1U, 4U})
    {
        horizontal_reaction +=
            solution.result.reactions[dof_map.global_index(node_id, DisplacementComponent::x)];
    }
    EXPECT_NEAR(horizontal_reaction, -10.0, tolerance);

    ASSERT_EQ(solution.result.element_results.size(), 2U);
    for (const auto &point : solution.result.element_results[0].gauss_points)
    {
        EXPECT_NEAR(point.strain[0], 0.01, tolerance);
        EXPECT_NEAR(point.stress[0], 10.0, tolerance);
    }
    for (const auto &point : solution.result.element_results[1].gauss_points)
    {
        EXPECT_NEAR(point.strain[0], 0.005, tolerance);
        EXPECT_NEAR(point.stress[0], 10.0, tolerance);
    }
}
} // namespace
