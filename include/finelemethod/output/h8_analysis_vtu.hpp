#pragma once

#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/h8_element_collection.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/solver/h8_analysis.hpp"

#include <filesystem>
#include <string>

namespace finelemethod::output
{
// Converts a complete H8 analysis result to ASCII VTK XML unstructured-grid
// content. Gauss-point results are averaged into element CellData.
[[nodiscard]] std::string create_h8_analysis_vtu(const model::NodeCollection &nodes,
                                                 const model::H8ElementCollection &elements,
                                                 const model::DofMap &dof_map,
                                                 const solver::H8AnalysisResult &analysis_result);

void write_h8_analysis_vtu(const std::filesystem::path &path, const model::NodeCollection &nodes,
                           const model::H8ElementCollection &elements, const model::DofMap &dof_map,
                           const solver::H8AnalysisResult &analysis_result);
} // namespace finelemethod::output
