#pragma once

#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/node.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace finelemethod::input
{
struct AbaqusNodeSet
{
    std::string name;
    std::vector<model::NodeId> node_ids;
};

// Parses explicit node-ID lists from *NSET sections. Repeated declarations of
// the same set append to that set in input order.
[[nodiscard]] std::vector<AbaqusNodeSet> parse_abaqus_node_sets(std::string_view input_text);
} // namespace finelemethod::input
