#pragma once

#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/q4_element_collection.hpp"
#include "finelemethod/solver/q4_plane_strain_analysis.hpp"
#include "finelemethod/solver/q4_plane_stress_analysis.hpp"

#include <filesystem>
#include <string>

namespace finelemethod::output
{
// Converts a complete Q4 plane-stress analysis result to ASCII VTU content.
[[nodiscard]] std::string create_q4_analysis_vtu(
    const model::NodeCollection &nodes, const model::Q4ElementCollection &elements,
    const model::DofMap &dof_map, const solver::Q4PlaneStressAnalysisResult &analysis_result);

// Converts a complete Q4 plane-strain analysis result to ASCII VTU content.
[[nodiscard]] std::string create_q4_analysis_vtu(
    const model::NodeCollection &nodes, const model::Q4ElementCollection &elements,
    const model::DofMap &dof_map, const solver::Q4PlaneStrainAnalysisResult &analysis_result);

void write_q4_analysis_vtu(const std::filesystem::path &path, const model::NodeCollection &nodes,
                           const model::Q4ElementCollection &elements, const model::DofMap &dof_map,
                           const solver::Q4PlaneStressAnalysisResult &analysis_result);

void write_q4_analysis_vtu(const std::filesystem::path &path, const model::NodeCollection &nodes,
                           const model::Q4ElementCollection &elements, const model::DofMap &dof_map,
                           const solver::Q4PlaneStrainAnalysisResult &analysis_result);
} // namespace finelemethod::output
