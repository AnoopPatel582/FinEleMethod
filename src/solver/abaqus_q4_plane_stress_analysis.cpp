#include "finelemethod/solver/abaqus_q4_plane_stress_analysis.hpp"

#include "finelemethod/input/abaqus_q4_model_parser.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/solver/q4_plane_stress_analysis.hpp"

namespace finelemethod::solver
{
Q4PlaneStressAnalysisResult solve_abaqus_q4_plane_stress(const std::string_view input_text)
{
    const input::AbaqusQ4Model input_model = input::parse_abaqus_q4_model(input_text);
    const model::DofMap dof_map(input_model.nodes, model::SpatialDimension::two_dimensional);

    return solve_q4_plane_stress_model(input_model.elements, input_model.nodes,
                                       input_model.materials, dof_map, input_model.point_loads,
                                       input_model.prescribed_displacements);
}
} // namespace finelemethod::solver
