#pragma once

#include "finelemethod/solver/q4_plane_stress_analysis.hpp"

#include <string_view>

namespace finelemethod::solver
{
// Parses supported ABAQUS input data and runs a complete Q4 plane-stress
// linear-static analysis.
[[nodiscard]] Q4PlaneStressAnalysisResult solve_abaqus_q4_plane_stress(std::string_view input_text);
} // namespace finelemethod::solver
