#pragma once

#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/node_collection.hpp"

#include <string_view>

namespace finelemethod::input
{
// Parses all *NODE sections from ABAQUS input text. Each data row must contain
// node ID, x, y, and an optional z coordinate.
[[nodiscard]] model::NodeCollection parse_abaqus_nodes(std::string_view input_text);
} // namespace finelemethod::input
