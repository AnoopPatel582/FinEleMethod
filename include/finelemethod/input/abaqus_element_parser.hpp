#pragma once

#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/q4_element.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace finelemethod::input
{
enum class Q4AnalysisType
{
    plane_stress,
    plane_strain,
};

struct AbaqusQ4Element
{
    model::ElementId id;
    model::Q4NodeIds node_ids;
    Q4AnalysisType analysis_type;
    std::string element_set;
};

// Parses all *ELEMENT, TYPE=CPS4 and TYPE=CPE4 sections. Material and thickness
// are resolved later from ABAQUS section definitions.
[[nodiscard]] std::vector<AbaqusQ4Element> parse_abaqus_q4_elements(std::string_view input_text);
} // namespace finelemethod::input
