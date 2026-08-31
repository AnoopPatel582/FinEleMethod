#pragma once

#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/h8_element.hpp"
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

struct AbaqusH8Element
{
    model::ElementId id;
    model::H8NodeIds node_ids;
    std::string element_set;
};

// Parses all *ELEMENT, TYPE=CPS4 and TYPE=CPE4 sections. Material and thickness
// are resolved later from ABAQUS section definitions.
[[nodiscard]] std::vector<AbaqusQ4Element> parse_abaqus_q4_elements(std::string_view input_text);

// Parses all *ELEMENT, TYPE=C3D8 sections. The material is resolved later from
// ABAQUS solid-section definitions.
[[nodiscard]] std::vector<AbaqusH8Element> parse_abaqus_h8_elements(std::string_view input_text);
} // namespace finelemethod::input
