#pragma once

#include "finelemethod/input/abaqus_node_set_parser.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/point_load.hpp"
#include "finelemethod/model/q4_element_collection.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"

#include <string_view>
#include <vector>

namespace finelemethod::input
{
struct AbaqusQ4Model
{
    model::NodeCollection nodes;
    model::MaterialCollection materials;
    model::Q4ElementCollection elements;
    std::vector<AbaqusNodeSet> node_sets;
    std::vector<model::PointLoad> point_loads;
    std::vector<solver::PrescribedDisplacement> prescribed_displacements;
};

// Parses and connects the supported ABAQUS plane-stress model data. Material
// IDs are assigned from one in the order that *MATERIAL definitions appear.
[[nodiscard]] AbaqusQ4Model parse_abaqus_q4_model(std::string_view input_text);
} // namespace finelemethod::input
