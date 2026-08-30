#include "finelemethod/solver/abaqus_q4_analysis.hpp"

#include "finelemethod/model/dof_map.hpp"

#include <utility>

namespace finelemethod::solver
{
AbaqusQ4Solution analyze_abaqus_q4(const std::string_view input_text)
{
    input::AbaqusQ4Model input_model = input::parse_abaqus_q4_model(input_text);
    const model::DofMap dof_map(input_model.nodes, model::SpatialDimension::two_dimensional);

    if (input_model.analysis_type == input::Q4AnalysisType::plane_stress)
    {
        auto result = solve_q4_plane_stress_model(
            input_model.elements, input_model.nodes, input_model.materials, dof_map,
            input_model.point_loads, input_model.pressure_loads,
            input_model.prescribed_displacements);
        return AbaqusQ4Solution{std::move(input_model), std::move(result)};
    }

    auto result = solve_q4_plane_strain_model(
        input_model.elements, input_model.nodes, input_model.materials, dof_map,
        input_model.point_loads, input_model.pressure_loads, input_model.prescribed_displacements);
    return AbaqusQ4Solution{std::move(input_model), std::move(result)};
}
} // namespace finelemethod::solver
