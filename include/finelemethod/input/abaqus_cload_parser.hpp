#pragma once

#include "finelemethod/input/abaqus_node_target.hpp"
#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/dof_map.hpp"

#include <string_view>
#include <vector>

namespace finelemethod::input
{
struct AbaqusConcentratedLoad
{
    AbaqusNodeTarget target;
    model::DisplacementComponent component;
    double magnitude;
};

// Parses node-based *CLOAD data. ABAQUS degree-of-freedom numbers 1, 2, and
// (for 3D) 3 map to x, y, and z respectively.
[[nodiscard]] std::vector<AbaqusConcentratedLoad> parse_abaqus_concentrated_loads(
    std::string_view input_text,
    model::SpatialDimension spatial_dimension = model::SpatialDimension::two_dimensional);
} // namespace finelemethod::input
