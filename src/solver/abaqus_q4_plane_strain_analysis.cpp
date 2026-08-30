#include "finelemethod/solver/abaqus_q4_plane_strain_analysis.hpp"

#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/dof_map.hpp"

#include <utility>

namespace finelemethod::solver
{
AbaqusQ4PlaneStrainSolution analyze_abaqus_q4_plane_strain(const std::string_view input_text)
{
    input::AbaqusQ4Model input_model = input::parse_abaqus_q4_model(input_text);
    if (input_model.analysis_type != input::Q4AnalysisType::plane_strain)
    {
        throw input::AbaqusParseError("Q4 plane-strain analysis requires ABAQUS CPE4 elements.");
    }
    const model::DofMap dof_map(input_model.nodes, model::SpatialDimension::two_dimensional);

    auto result = solve_q4_plane_strain_model(
        input_model.elements, input_model.nodes, input_model.materials, dof_map,
        input_model.point_loads, input_model.pressure_loads, input_model.prescribed_displacements);
    return AbaqusQ4PlaneStrainSolution{std::move(input_model), std::move(result)};
}

Q4PlaneStrainAnalysisResult solve_abaqus_q4_plane_strain(const std::string_view input_text)
{
    auto solution = analyze_abaqus_q4_plane_strain(input_text);
    return std::move(solution.result);
}
} // namespace finelemethod::solver
