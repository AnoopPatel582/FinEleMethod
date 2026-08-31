#pragma once

#include "finelemethod/input/abaqus_node_set_parser.hpp"
#include "finelemethod/model/h8_element_collection.hpp"
#include "finelemethod/model/h8_face_pressure_load.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/point_load.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"

#include <string_view>
#include <vector>

namespace finelemethod::input
{
struct AbaqusH8Model
{
    model::NodeCollection nodes;
    model::MaterialCollection materials;
    model::H8ElementCollection elements;
    std::vector<AbaqusNodeSet> node_sets;
    std::vector<model::PointLoad> point_loads;
    std::vector<model::H8FacePressureLoad> pressure_loads;
    std::vector<solver::PrescribedDisplacement> prescribed_displacements;
};

// Parses and connects the supported ABAQUS C3D8 model data. Material IDs are
// assigned from one in the order that *MATERIAL definitions appear.
[[nodiscard]] AbaqusH8Model parse_abaqus_h8_model(std::string_view input_text);
} // namespace finelemethod::input
