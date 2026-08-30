#include "finelemethod/solver/abaqus_q4_plane_stress_analysis.hpp"

#include "finelemethod/input/abaqus_q4_model_parser.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/solver/q4_plane_stress_analysis.hpp"

#include <utility>

namespace finelemethod::solver
{
AbaqusQ4PlaneStressSolution analyze_abaqus_q4_plane_stress(const std::string_view input_text)
{
    input::AbaqusQ4Model input_model = input::parse_abaqus_q4_model(input_text);
    const model::DofMap dof_map(input_model.nodes, model::SpatialDimension::two_dimensional);

    auto result = solve_q4_plane_stress_model(
        input_model.elements, input_model.nodes, input_model.materials, dof_map,
        input_model.point_loads, input_model.pressure_loads, input_model.prescribed_displacements);
    return AbaqusQ4PlaneStressSolution{std::move(input_model), std::move(result)};
}

Q4PlaneStressAnalysisResult solve_abaqus_q4_plane_stress(const std::string_view input_text)
{
    auto solution = analyze_abaqus_q4_plane_stress(input_text);
    return std::move(solution.result);
}
} // namespace finelemethod::solver
