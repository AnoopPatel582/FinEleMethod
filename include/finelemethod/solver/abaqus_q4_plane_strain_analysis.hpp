#pragma once

#include "finelemethod/input/abaqus_q4_model_parser.hpp"
#include "finelemethod/solver/q4_plane_strain_analysis.hpp"

#include <string_view>

namespace finelemethod::solver
{
struct AbaqusQ4PlaneStrainSolution
{
    input::AbaqusQ4Model model;
    Q4PlaneStrainAnalysisResult result;
};

// Parses supported ABAQUS input data and retains both the validated model and
// its complete Q4 plane-strain solution.
[[nodiscard]] AbaqusQ4PlaneStrainSolution analyze_abaqus_q4_plane_strain(
    std::string_view input_text);

// Parses supported ABAQUS input data and runs a complete Q4 plane-strain
// linear-static analysis.
[[nodiscard]] Q4PlaneStrainAnalysisResult solve_abaqus_q4_plane_strain(std::string_view input_text);
} // namespace finelemethod::solver
