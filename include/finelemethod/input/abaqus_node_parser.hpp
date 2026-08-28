#pragma once

#include "finelemethod/model/node_collection.hpp"

#include <stdexcept>
#include <string_view>

namespace finelemethod::input
{
class AbaqusParseError : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

// Parses all *NODE sections from ABAQUS input text. Each data row must contain
// node ID, x, y, and an optional z coordinate.
[[nodiscard]] model::NodeCollection parse_abaqus_nodes(std::string_view input_text);
} // namespace finelemethod::input
