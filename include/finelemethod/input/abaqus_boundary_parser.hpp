#pragma once

#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/node.hpp"

#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace finelemethod::input
{
using AbaqusBoundaryTarget = std::variant<model::NodeId, std::string>;

struct AbaqusNodalDisplacement
{
    AbaqusBoundaryTarget target;
    model::DisplacementComponent component;
    double value;
};

// Parses node-based *BOUNDARY displacement data for a two-dimensional model.
// ABAQUS degree-of-freedom numbers 1 and 2 map to x and y respectively.
[[nodiscard]] std::vector<AbaqusNodalDisplacement> parse_abaqus_nodal_displacements(
    std::string_view input_text);
} // namespace finelemethod::input
