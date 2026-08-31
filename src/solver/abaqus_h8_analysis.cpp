#include "finelemethod/solver/abaqus_h8_analysis.hpp"

#include "finelemethod/input/abaqus_h8_model_parser.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/solver/h8_analysis.hpp"

#include <utility>

namespace finelemethod::solver
{
AbaqusH8Solution analyze_abaqus_h8(const std::string_view input_text)
{
    input::AbaqusH8Model input_model = input::parse_abaqus_h8_model(input_text);
    const model::DofMap dof_map(input_model.nodes, model::SpatialDimension::three_dimensional);

    auto result =
        solve_h8_model(input_model.elements, input_model.nodes, input_model.materials, dof_map,
                       input_model.point_loads, input_model.prescribed_displacements);
    return AbaqusH8Solution{std::move(input_model), std::move(result)};
}

H8AnalysisResult solve_abaqus_h8(const std::string_view input_text)
{
    auto solution = analyze_abaqus_h8(input_text);
    return std::move(solution.result);
}
} // namespace finelemethod::solver
