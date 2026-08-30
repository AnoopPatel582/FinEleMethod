#pragma once

#include "finelemethod/input/abaqus_q4_model_parser.hpp"
#include "finelemethod/solver/q4_plane_strain_analysis.hpp"
#include "finelemethod/solver/q4_plane_stress_analysis.hpp"

#include <string_view>
#include <variant>

namespace finelemethod::solver
{
using AbaqusQ4AnalysisResult =
    std::variant<Q4PlaneStressAnalysisResult, Q4PlaneStrainAnalysisResult>;

struct AbaqusQ4Solution
{
    input::AbaqusQ4Model model;
    AbaqusQ4AnalysisResult result;
};

// Parses the ABAQUS model once and dispatches CPS4 to plane stress or CPE4 to
// plane strain according to the explicit element formulation.
[[nodiscard]] AbaqusQ4Solution analyze_abaqus_q4(std::string_view input_text);
} // namespace finelemethod::solver
