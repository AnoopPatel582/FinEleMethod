#pragma once

#include "finelemethod/input/abaqus_q4_model_parser.hpp"
#include "finelemethod/solver/q4_plane_stress_analysis.hpp"

#include <string_view>

namespace finelemethod::solver
{
struct AbaqusQ4PlaneStressSolution
{
    input::AbaqusQ4Model model;
    Q4PlaneStressAnalysisResult result;
};

// Parses supported ABAQUS input data and retains both the validated model and
// its complete Q4 plane-stress solution.
[[nodiscard]] AbaqusQ4PlaneStressSolution analyze_abaqus_q4_plane_stress(
    std::string_view input_text);

// Parses supported ABAQUS input data and runs a complete Q4 plane-stress
// linear-static analysis.
[[nodiscard]] Q4PlaneStressAnalysisResult solve_abaqus_q4_plane_stress(std::string_view input_text);
} // namespace finelemethod::solver
