#pragma once

#include "finelemethod/input/abaqus_parse_error.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace finelemethod::input
{
struct AbaqusSolidSection
{
    std::string element_set;
    std::string material_name;
    double thickness;
};

// Parses *SOLID SECTION definitions used to assign material and plane-stress
// thickness to an element set.
[[nodiscard]] std::vector<AbaqusSolidSection> parse_abaqus_solid_sections(
    std::string_view input_text);
} // namespace finelemethod::input
