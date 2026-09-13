#pragma once

#include "finelemethod/model/element_id.hpp"
#include "finelemethod/model/node.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace finelemethod::input
{
enum class AbaqusElementType
{
    cps3,
    cps4,
    cps4r,
    cpe4,
    c3d4,
    c3d8,
    c3d8r,
};

struct AbaqusImportedNode
{
    model::NodeId id;
    std::array<double, 3> coordinates;
};

struct AbaqusImportedElement
{
    model::ElementId id;
    AbaqusElementType type;
    std::vector<model::NodeId> node_ids;
    std::string material_name;
    double section_thickness = 1.0;
};

struct AbaqusImportedDisplacement
{
    model::NodeId node_id;
    std::size_t component;
    double value;
};

struct AbaqusImportedPointLoad
{
    model::NodeId node_id;
    std::size_t component;
    double magnitude;
};

struct AbaqusImportedPressure
{
    model::ElementId element_id;
    std::size_t face;
    double magnitude;
};

struct AbaqusImportedModel
{
    AbaqusElementType element_type = AbaqusElementType::cps4;
    std::vector<AbaqusImportedNode> nodes;
    std::vector<AbaqusImportedElement> elements;
    std::size_t material_count{};
    std::vector<AbaqusImportedDisplacement> prescribed_displacements;
    std::vector<AbaqusImportedPointLoad> point_loads;
    std::vector<AbaqusImportedPressure> pressure_loads;
};

// Imports the supported linear-static subset of either a flat ABAQUS deck or a
// single-part, single-untransformed-instance ABAQUS/CAE deck. Part and assembly
// set namespaces are resolved before loads and constraints are expanded.
[[nodiscard]] AbaqusImportedModel import_abaqus_cae_model(std::string_view input_text);

[[nodiscard]] std::string_view abaqus_element_type_name(AbaqusElementType type) noexcept;
[[nodiscard]] std::size_t abaqus_element_dimension(AbaqusElementType type) noexcept;
} // namespace finelemethod::input
