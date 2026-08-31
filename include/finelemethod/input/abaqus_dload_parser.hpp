#pragma once

#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/h8_face_pressure_load.hpp"
#include "finelemethod/model/q4_edge_pressure_load.hpp"

#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace finelemethod::input
{
using AbaqusElementTarget = std::variant<model::ElementId, std::string>;

struct AbaqusQ4EdgePressure
{
    AbaqusElementTarget target;
    model::Q4Edge edge;
    double pressure;
};

struct AbaqusH8FacePressure
{
    AbaqusElementTarget target;
    model::H8Face face;
    double pressure;
};

// Parses uniform P1-P4 edge pressures from *DLOAD sections. Targets may be
// direct element IDs or element-set names.
[[nodiscard]] std::vector<AbaqusQ4EdgePressure> parse_abaqus_q4_edge_pressures(
    std::string_view input_text);

// Parses uniform P1-P6 face pressures from *DLOAD sections. Targets may be
// direct element IDs or element-set names.
[[nodiscard]] std::vector<AbaqusH8FacePressure> parse_abaqus_h8_face_pressures(
    std::string_view input_text);
} // namespace finelemethod::input
