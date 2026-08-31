#include "finelemethod/solver/h8_analysis.hpp"

#include "finelemethod/assembly/h8_pressure_assembly.hpp"
#include "finelemethod/assembly/h8_stiffness_assembly.hpp"
#include "finelemethod/assembly/load_assembly.hpp"
#include "finelemethod/postprocessing/h8_results.hpp"

#include <utility>

namespace finelemethod::solver
{
H8AnalysisResult solve_h8_model(
    const model::H8ElementCollection &elements, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map,
    const std::span<const model::PointLoad> point_loads,
    const std::span<const model::H8FacePressureLoad> pressure_loads,
    const std::span<const PrescribedDisplacement> prescribed_displacements)
{
    const math::DenseMatrix stiffness_matrix =
        assembly::assemble_h8_stiffness(elements, nodes, materials, dof_map);
    math::DenseVector load_vector = assembly::assemble_point_load_vector(dof_map, point_loads);
    load_vector +=
        assembly::assemble_h8_face_pressure_loads(elements, nodes, dof_map, pressure_loads);
    auto static_result =
        solve_dense_static_system(stiffness_matrix, load_vector, prescribed_displacements);
    auto element_results = postprocessing::recover_h8_model_results(
        elements, nodes, materials, dof_map, static_result.displacements);

    return H8AnalysisResult{
        std::move(static_result.displacements),
        std::move(static_result.reactions),
        std::move(element_results),
    };
}
} // namespace finelemethod::solver
