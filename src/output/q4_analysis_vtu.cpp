#include "finelemethod/output/q4_analysis_vtu.hpp"

#include "finelemethod/output/q4_vtu_writer.hpp"

namespace finelemethod::output
{
std::string create_q4_analysis_vtu(const model::NodeCollection &nodes,
                                   const model::Q4ElementCollection &elements,
                                   const model::DofMap &dof_map,
                                   const solver::Q4PlaneStressAnalysisResult &analysis_result)
{
    return create_q4_results_vtu(nodes, elements, dof_map, analysis_result.displacements,
                                 analysis_result.reactions, analysis_result.element_results);
}

void write_q4_analysis_vtu(const std::filesystem::path &path, const model::NodeCollection &nodes,
                           const model::Q4ElementCollection &elements, const model::DofMap &dof_map,
                           const solver::Q4PlaneStressAnalysisResult &analysis_result)
{
    write_q4_results_vtu(path, nodes, elements, dof_map, analysis_result.displacements,
                         analysis_result.reactions, analysis_result.element_results);
}
} // namespace finelemethod::output
