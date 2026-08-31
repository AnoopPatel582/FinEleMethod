#include "finelemethod/input/abaqus_input_file.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/solver/abaqus_q4_plane_stress_analysis.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>

namespace
{
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::SpatialDimension;

TEST(Q4CantileverBenchmark, MatchesBeamTheoryWithinDocumentedMeshTolerance)
{
    const std::filesystem::path input_path = std::filesystem::path(FINELEMETHOD_SOURCE_DIR) /
                                             "examples" / "abaqus" / "q4_cantilever.inp";
    const std::string input_text = finelemethod::input::read_abaqus_input_file(input_path);
    const auto solution = finelemethod::solver::analyze_abaqus_q4_plane_stress(input_text);
    const DofMap dof_map(solution.model.nodes, SpatialDimension::two_dimensional);

    constexpr double load = 1.0;
    constexpr double length = 10.0;
    constexpr double youngs_modulus = 1000.0;
    constexpr double second_moment_of_area = 1.0 / 12.0;
    constexpr double analytical_tip_displacement =
        -load * length * length * length / (3.0 * youngs_modulus * second_moment_of_area);
    const double numerical_tip_displacement =
        solution.result.displacements[dof_map.global_index(63, DisplacementComponent::y)];
    const double relative_error = std::abs(
        (numerical_tip_displacement - analytical_tip_displacement) / analytical_tip_displacement);

    EXPECT_LT(relative_error, 0.10);

    double vertical_support_reaction = 0.0;
    for (const std::size_t node_id : {1U, 22U, 43U, 64U, 85U})
    {
        vertical_support_reaction +=
            solution.result.reactions[dof_map.global_index(node_id, DisplacementComponent::y)];
    }
    EXPECT_NEAR(vertical_support_reaction, load, 1.0e-10);
}
} // namespace
