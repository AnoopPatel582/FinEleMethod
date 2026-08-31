#pragma once

#include "finelemethod/input/abaqus_node_target.hpp"
#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/node.hpp"

#include <string_view>
#include <vector>

namespace finelemethod::input
{
struct AbaqusNodalDisplacement
{
    AbaqusNodeTarget target;
    model::DisplacementComponent component;
    double value;
};

// Parses node-based *BOUNDARY displacement data. ABAQUS degree-of-freedom
// numbers 1, 2, and (for 3D) 3 map to x, y, and z respectively.
[[nodiscard]] std::vector<AbaqusNodalDisplacement> parse_abaqus_nodal_displacements(
    std::string_view input_text,
    model::SpatialDimension spatial_dimension = model::SpatialDimension::two_dimensional);
} // namespace finelemethod::input
