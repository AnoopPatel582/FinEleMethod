#pragma once

#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/point_load.hpp"

#include <string_view>
#include <vector>

namespace finelemethod::input
{
// Parses node-based *CLOAD data for a two-dimensional model. ABAQUS
// degree-of-freedom numbers 1 and 2 map to x and y respectively.
[[nodiscard]] std::vector<model::PointLoad> parse_abaqus_point_loads(std::string_view input_text);
} // namespace finelemethod::input
