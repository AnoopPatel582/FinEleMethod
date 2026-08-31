#include "finelemethod/elements/q4_shape_functions.hpp"
#include "finelemethod/input/abaqus_input_file.hpp"
#include "finelemethod/solver/abaqus_q4_plane_stress_analysis.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <filesystem>
#include <limits>

namespace
{
TEST(Q4PlateWithHoleBenchmark, MatchesKirschHoopStressNearHole)
{
    const std::filesystem::path input_path = std::filesystem::path(FINELEMETHOD_SOURCE_DIR) /
                                             "examples" / "abaqus" / "q4_plate_with_hole.inp";
    const std::string input_text = finelemethod::input::read_abaqus_input_file(input_path);
    const auto solution = finelemethod::solver::analyze_abaqus_q4_plane_stress(input_text);

    constexpr std::size_t hole_crown_element_id = 16;
    const auto &element = solution.model.elements.at(hole_crown_element_id);
    const auto &element_results = solution.result.element_results[hole_crown_element_id - 1];

    double closest_distance = std::numeric_limits<double>::max();
    double numerical_hoop_stress = 0.0;
    double analytical_hoop_stress = 0.0;

    for (const auto &point_result : element_results.gauss_points)
    {
        const auto shape =
            finelemethod::elements::q4_shape_functions(point_result.xi, point_result.eta);
        double x = 0.0;
        double y = 0.0;
        for (std::size_t node_index = 0; node_index < element.node_ids().size(); ++node_index)
        {
            const auto &node = solution.model.nodes.at(element.node_ids()[node_index]);
            x += shape[node_index] * node.x();
            y += shape[node_index] * node.y();
        }

        const double distance_to_hole_crown = std::hypot(x, y - 1.0);
        if (distance_to_hole_crown >= closest_distance)
        {
            continue;
        }

        closest_distance = distance_to_hole_crown;
        const double radius = std::hypot(x, y);
        const double theta = std::atan2(y, x);
        const double sine = std::sin(theta);
        const double cosine = std::cos(theta);
        const auto &stress = point_result.stress;
        numerical_hoop_stress =
            stress[0] * sine * sine + stress[1] * cosine * cosine - 2.0 * stress[2] * sine * cosine;

        constexpr double hole_radius = 1.0;
        constexpr double remote_stress = 1.0;
        const double radius_ratio_squared = hole_radius * hole_radius / (radius * radius);
        const double radius_ratio_fourth = radius_ratio_squared * radius_ratio_squared;
        analytical_hoop_stress =
            remote_stress * 0.5 * (1.0 + radius_ratio_squared) -
            remote_stress * 0.5 * (1.0 + 3.0 * radius_ratio_fourth) * std::cos(2.0 * theta);
    }

    const double relative_error =
        std::abs((numerical_hoop_stress - analytical_hoop_stress) / analytical_hoop_stress);
    EXPECT_LT(relative_error, 0.10)
        << "numerical=" << numerical_hoop_stress << ", analytical=" << analytical_hoop_stress;
}
} // namespace
