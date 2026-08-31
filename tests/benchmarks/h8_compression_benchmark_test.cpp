#include "finelemethod/input/abaqus_input_file.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/solver/abaqus_h8_analysis.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::SpatialDimension;

TEST(H8CompressionBenchmark, MatchesUniaxialElasticitySolution)
{
    const std::filesystem::path input_path = std::filesystem::path(FINELEMETHOD_SOURCE_DIR) /
                                             "examples" / "abaqus" / "h8_compression.inp";
    const std::string input_text = finelemethod::input::read_abaqus_input_file(input_path);
    const auto solution = finelemethod::solver::analyze_abaqus_h8(input_text);
    const DofMap dof_map(solution.model.nodes, SpatialDimension::three_dimensional);

    constexpr double tolerance = 1.0e-11;
    EXPECT_NEAR(solution.result.displacements[dof_map.global_index(7, DisplacementComponent::z)],
                -0.01, tolerance);
    EXPECT_NEAR(solution.result.displacements[dof_map.global_index(7, DisplacementComponent::x)],
                0.0025, tolerance);
    EXPECT_NEAR(solution.result.displacements[dof_map.global_index(7, DisplacementComponent::y)],
                0.0025, tolerance);

    double bottom_reaction = 0.0;
    for (const std::size_t node_id : {1U, 2U, 3U, 4U})
    {
        bottom_reaction +=
            solution.result.reactions[dof_map.global_index(node_id, DisplacementComponent::z)];
    }
    EXPECT_NEAR(bottom_reaction, 10.0, tolerance);

    ASSERT_EQ(solution.result.element_results.size(), 1U);
    for (const auto &point : solution.result.element_results.front().gauss_points)
    {
        EXPECT_NEAR(point.strain[0], 0.0025, tolerance);
        EXPECT_NEAR(point.strain[1], 0.0025, tolerance);
        EXPECT_NEAR(point.strain[2], -0.01, tolerance);
        EXPECT_NEAR(point.stress[0], 0.0, tolerance);
        EXPECT_NEAR(point.stress[1], 0.0, tolerance);
        EXPECT_NEAR(point.stress[2], -10.0, tolerance);
        EXPECT_NEAR(point.von_mises, 10.0, tolerance);
        EXPECT_NEAR(point.principal_stresses[0], 0.0, tolerance);
        EXPECT_NEAR(point.principal_stresses[1], 0.0, tolerance);
        EXPECT_NEAR(point.principal_stresses[2], -10.0, tolerance);
    }
}
} // namespace
