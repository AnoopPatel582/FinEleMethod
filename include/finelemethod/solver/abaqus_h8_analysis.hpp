#pragma once

#include "finelemethod/input/abaqus_h8_model_parser.hpp"
#include "finelemethod/solver/h8_analysis.hpp"

#include <string_view>

namespace finelemethod::solver
{
struct AbaqusH8Solution
{
    input::AbaqusH8Model model;
    H8AnalysisResult result;
};

// Parses supported ABAQUS C3D8 input data and retains both the validated model
// and its complete H8 solution.
[[nodiscard]] AbaqusH8Solution analyze_abaqus_h8(std::string_view input_text);

// Parses supported ABAQUS C3D8 input data and runs a complete H8 linear-static
// analysis.
[[nodiscard]] H8AnalysisResult solve_abaqus_h8(std::string_view input_text);
} // namespace finelemethod::solver
