#include "finelemethod/solver/q4_plane_strain_analysis.hpp"

#include "finelemethod/assembly/load_assembly.hpp"
#include "finelemethod/assembly/q4_pressure_assembly.hpp"
#include "finelemethod/assembly/q4_stiffness_assembly.hpp"
#include "finelemethod/postprocessing/q4_plane_strain_results.hpp"

#include <utility>

namespace finelemethod::solver
{
Q4PlaneStrainAnalysisResult solve_q4_plane_strain_model(
    const model::Q4ElementCollection &elements, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map,
    const std::span<const model::PointLoad> point_loads,
    const std::span<const model::Q4EdgePressureLoad> pressure_loads,
    const std::span<const PrescribedDisplacement> prescribed_displacements)
{
    const auto stiffness_matrix =
        assembly::assemble_q4_plane_strain_stiffness(elements, nodes, materials, dof_map);
    auto load_vector = assembly::assemble_point_load_vector(dof_map, point_loads);
    load_vector +=
        assembly::assemble_q4_edge_pressure_loads(elements, nodes, dof_map, pressure_loads);

    auto static_result =
        solve_dense_static_system(stiffness_matrix, load_vector, prescribed_displacements);
    auto element_results = postprocessing::recover_q4_plane_strain_model_results(
        elements, nodes, materials, dof_map, static_result.displacements);

    return Q4PlaneStrainAnalysisResult{
        std::move(static_result.displacements),
        std::move(static_result.reactions),
        std::move(element_results),
    };
}
} // namespace finelemethod::solver
