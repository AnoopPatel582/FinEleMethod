#include "finelemethod/solver/q4_plane_stress_analysis.hpp"

#include "finelemethod/assembly/load_assembly.hpp"
#include "finelemethod/assembly/q4_stiffness_assembly.hpp"

namespace finelemethod::solver
{
DenseStaticResult solve_q4_plane_stress_model(
    const model::Q4ElementCollection &elements, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map,
    const std::span<const model::PointLoad> point_loads,
    const std::span<const PrescribedDisplacement> prescribed_displacements)
{
    const auto stiffness_matrix =
        assembly::assemble_q4_plane_stress_stiffness(elements, nodes, materials, dof_map);
    const auto load_vector = assembly::assemble_point_load_vector(dof_map, point_loads);

    return solve_dense_static_system(stiffness_matrix, load_vector, prescribed_displacements);
}
} // namespace finelemethod::solver
