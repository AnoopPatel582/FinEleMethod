#pragma once

#include <cstddef>
#include <string_view>

namespace finelemethod::input
{
enum class AbaqusAnalysisType
{
    q4_plane_stress,
    q4_plane_strain,
    h8_three_dimensional,
};

struct AbaqusModelSummary
{
    AbaqusAnalysisType analysis_type = AbaqusAnalysisType::q4_plane_stress;
    std::size_t node_count{};
    std::size_t element_count{};
    std::size_t material_count{};
    std::size_t prescribed_displacement_count{};
    std::size_t point_load_count{};
    std::size_t pressure_load_count{};
};

// Parses and validates the supported ABAQUS model without solving it, then
// returns the model information needed by user interfaces.
[[nodiscard]] AbaqusModelSummary summarize_abaqus_model(std::string_view input_text);
} // namespace finelemethod::input
