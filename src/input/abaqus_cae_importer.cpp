#include "finelemethod/input/abaqus_cae_importer.hpp"

#include "abaqus_parser_utilities.hpp"
#include "finelemethod/input/abaqus_material_parser.hpp"
#include "finelemethod/input/abaqus_parse_error.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace finelemethod::input
{
namespace
{
struct DataRow
{
    std::size_t line_number;
    std::vector<std::string> fields;
};

struct KeywordBlock
{
    std::string keyword;
    std::unordered_map<std::string, std::string> parameters;
    std::unordered_set<std::string> flags;
    std::vector<DataRow> rows;
    std::size_t line_number{};
    std::string part_name;
    bool in_assembly{};
};

struct SurfaceEntry
{
    std::string element_set;
    std::size_t face;
    std::string part_name;
    bool in_assembly{};
};

std::string uppercase_copy(const std::string_view text)
{
    std::string result(text);
    std::ranges::transform(result, result.begin(), [](const unsigned char character) {
        return static_cast<char>(std::toupper(character));
    });
    return result;
}

std::string require_parameter(const KeywordBlock &block, const std::string_view name)
{
    const auto parameter = block.parameters.find(uppercase_copy(name));
    if (parameter == block.parameters.end() || parameter->second.empty())
    {
        throw AbaqusParseError("ABAQUS *" + block.keyword + " requires " + std::string(name) +
                               " on line " + std::to_string(block.line_number) + ".");
    }
    return parameter->second;
}

KeywordBlock parse_keyword(const std::string_view line, const std::size_t line_number,
                           const std::string &part_name, const bool in_assembly)
{
    const auto fields = detail::split_fields(line);
    KeywordBlock block;
    block.keyword = uppercase_copy(detail::trim(fields.front().substr(1)));
    block.line_number = line_number;
    block.part_name = part_name;
    block.in_assembly = in_assembly;
    for (std::size_t index = 1; index < fields.size(); ++index)
    {
        const std::size_t equals = fields[index].find('=');
        if (equals == std::string_view::npos)
        {
            block.flags.insert(uppercase_copy(detail::trim(fields[index])));
            continue;
        }
        const std::string name = uppercase_copy(detail::trim(fields[index].substr(0, equals)));
        const std::string value(detail::trim(fields[index].substr(equals + 1)));
        block.parameters.insert_or_assign(name, value);
    }
    return block;
}

std::vector<KeywordBlock> parse_blocks(const std::string_view input_text)
{
    std::vector<KeywordBlock> blocks;
    std::optional<std::size_t> current_block;
    std::string current_part;
    bool in_assembly = false;
    std::size_t line_number = 0;
    std::size_t line_start = 0;

    while (line_start <= input_text.size())
    {
        ++line_number;
        const std::size_t line_end = input_text.find('\n', line_start);
        std::string_view line = line_end == std::string_view::npos
                                    ? input_text.substr(line_start)
                                    : input_text.substr(line_start, line_end - line_start);
        if (!line.empty() && line.back() == '\r')
        {
            line.remove_suffix(1);
        }
        line = detail::trim(line);

        if (line.starts_with("**"))
        {
            // ABAQUS comment line.
        }
        else if (line.starts_with('*'))
        {
            KeywordBlock block = parse_keyword(line, line_number, current_part, in_assembly);
            if (block.keyword == "PART")
            {
                if (!current_part.empty())
                {
                    throw AbaqusParseError("Nested ABAQUS *PART on line " +
                                           std::to_string(line_number) + ".");
                }
                current_part = require_parameter(block, "NAME");
                block.part_name = current_part;
            }
            else if (block.keyword == "ASSEMBLY")
            {
                in_assembly = true;
                block.in_assembly = true;
            }

            blocks.push_back(std::move(block));
            current_block = blocks.size() - 1;

            if (blocks.back().keyword == "END PART")
            {
                current_part.clear();
            }
            else if (blocks.back().keyword == "END ASSEMBLY")
            {
                in_assembly = false;
            }
        }
        else if (!line.empty() && current_block.has_value())
        {
            const auto fields = detail::split_fields(line);
            DataRow row{line_number, {}};
            row.fields.reserve(fields.size());
            for (const std::string_view field : fields)
            {
                row.fields.emplace_back(detail::trim(field));
            }
            blocks[*current_block].rows.push_back(std::move(row));
        }

        if (line_end == std::string_view::npos)
        {
            break;
        }
        line_start = line_end + 1;
    }
    return blocks;
}

AbaqusElementType parse_element_type(const std::string_view value, const std::size_t line_number)
{
    if (detail::equals_case_insensitive(value, "CPS3"))
    {
        return AbaqusElementType::cps3;
    }
    if (detail::equals_case_insensitive(value, "CPS4"))
    {
        return AbaqusElementType::cps4;
    }
    if (detail::equals_case_insensitive(value, "CPS4R"))
    {
        return AbaqusElementType::cps4r;
    }
    if (detail::equals_case_insensitive(value, "CPE4"))
    {
        return AbaqusElementType::cpe4;
    }
    if (detail::equals_case_insensitive(value, "C3D4"))
    {
        return AbaqusElementType::c3d4;
    }
    if (detail::equals_case_insensitive(value, "C3D8"))
    {
        return AbaqusElementType::c3d8;
    }
    if (detail::equals_case_insensitive(value, "C3D8R"))
    {
        return AbaqusElementType::c3d8r;
    }
    throw AbaqusParseError("Unsupported ABAQUS element TYPE='" + std::string(value) + "' on line " +
                           std::to_string(line_number) + ".");
}

std::size_t expected_node_count(const AbaqusElementType type) noexcept
{
    switch (type)
    {
    case AbaqusElementType::cps3:
        return 3;
    case AbaqusElementType::cps4:
    case AbaqusElementType::cps4r:
    case AbaqusElementType::cpe4:
    case AbaqusElementType::c3d4:
        return 4;
    case AbaqusElementType::c3d8:
    case AbaqusElementType::c3d8r:
        return 8;
    }
    return 0;
}

std::string scope_key(const std::string_view category, const std::string_view part_name,
                      const std::string_view name)
{
    return std::string(category) + ':' + uppercase_copy(part_name) + ':' + uppercase_copy(name);
}

std::string block_scope_key(const KeywordBlock &block, const std::string_view name)
{
    if (block.in_assembly)
    {
        return scope_key("ASSEMBLY", {}, name);
    }
    if (!block.part_name.empty())
    {
        return scope_key("PART", block.part_name, name);
    }
    return scope_key("GLOBAL", {}, name);
}

template <typename Id>
std::vector<Id> parse_set_ids(const KeywordBlock &block, const std::string_view description)
{
    std::vector<Id> values;
    for (const DataRow &row : block.rows)
    {
        for (const std::string &field : row.fields)
        {
            if (!field.empty())
            {
                values.push_back(detail::parse_number<Id>(field, row.line_number, description));
            }
        }
    }
    if (values.empty())
    {
        throw AbaqusParseError("ABAQUS *" + block.keyword + " on line " +
                               std::to_string(block.line_number) + " contains no IDs.");
    }
    if (!block.flags.contains("GENERATE"))
    {
        return values;
    }
    if ((values.size() != 2 && values.size() != 3) || values[0] > values[1])
    {
        throw AbaqusParseError("Invalid generated ABAQUS set on line " +
                               std::to_string(block.line_number) + ".");
    }
    const Id increment = values.size() == 3 ? values[2] : 1;
    if (increment == 0 || (values[1] - values[0]) % increment != 0)
    {
        throw AbaqusParseError("Invalid generated ABAQUS set on line " +
                               std::to_string(block.line_number) + ".");
    }
    std::vector<Id> generated;
    for (Id id = values[0];; id += increment)
    {
        generated.push_back(id);
        if (id == values[1])
        {
            break;
        }
    }
    return generated;
}

template <typename Id>
void append_unique(std::unordered_map<std::string, std::vector<Id>> &sets, const std::string &key,
                   const std::vector<Id> &ids, const std::size_t line_number)
{
    std::vector<Id> &target = sets[key];
    std::unordered_set<Id> existing(target.begin(), target.end());
    for (const Id id : ids)
    {
        if (!existing.insert(id).second)
        {
            throw AbaqusParseError("Duplicate ID in ABAQUS set on line " +
                                   std::to_string(line_number) + ".");
        }
        target.push_back(id);
    }
}

template <typename Id>
const std::vector<Id> &resolve_set(const std::unordered_map<std::string, std::vector<Id>> &sets,
                                   const std::string_view name, const std::string_view part_name,
                                   const bool assembly_context)
{
    std::vector<std::string> candidates;
    if (assembly_context)
    {
        candidates.push_back(scope_key("ASSEMBLY", {}, name));
    }
    candidates.push_back(scope_key("GLOBAL", {}, name));
    if (!part_name.empty())
    {
        candidates.push_back(scope_key("PART", part_name, name));
    }
    for (const std::string &candidate : candidates)
    {
        const auto set = sets.find(candidate);
        if (set != sets.end())
        {
            return set->second;
        }
    }
    throw AbaqusParseError("ABAQUS reference to unknown set '" + std::string(name) + "'.");
}

std::size_t parse_face(const std::string_view value, const std::size_t line_number)
{
    if (value.size() < 2 || (value.front() != 'S' && value.front() != 's') &&
                                (value.front() != 'P' && value.front() != 'p'))
    {
        throw AbaqusParseError("Unsupported ABAQUS surface face '" + std::string(value) +
                               "' on line " + std::to_string(line_number) + ".");
    }
    return detail::parse_number<std::size_t>(value.substr(1), line_number, "surface face");
}

template <typename Id>
std::vector<Id> resolve_numeric_or_set(const std::string &target,
                                       const std::unordered_map<std::string, std::vector<Id>> &sets,
                                       const std::string &part_name, const bool assembly_context,
                                       const std::size_t line_number,
                                       const std::string_view description)
{
    try
    {
        return {detail::parse_number<Id>(target, line_number, description)};
    }
    catch (const AbaqusParseError &)
    {
        return resolve_set(sets, target, part_name, assembly_context);
    }
}
} // namespace

std::string_view abaqus_element_type_name(const AbaqusElementType type) noexcept
{
    switch (type)
    {
    case AbaqusElementType::cps3:
        return "CPS3";
    case AbaqusElementType::cps4:
        return "CPS4";
    case AbaqusElementType::cps4r:
        return "CPS4R";
    case AbaqusElementType::cpe4:
        return "CPE4";
    case AbaqusElementType::c3d4:
        return "C3D4";
    case AbaqusElementType::c3d8:
        return "C3D8";
    case AbaqusElementType::c3d8r:
        return "C3D8R";
    }
    return "unknown";
}

std::size_t abaqus_element_dimension(const AbaqusElementType type) noexcept
{
    switch (type)
    {
    case AbaqusElementType::cps3:
    case AbaqusElementType::cps4:
    case AbaqusElementType::cps4r:
    case AbaqusElementType::cpe4:
        return 2;
    case AbaqusElementType::c3d4:
    case AbaqusElementType::c3d8:
    case AbaqusElementType::c3d8r:
        return 3;
    }
    return 0;
}

AbaqusImportedModel import_abaqus_cae_model(const std::string_view input_text)
{
    const std::vector<KeywordBlock> blocks = parse_blocks(input_text);
    AbaqusImportedModel model;
    std::unordered_set<std::string> parts;
    std::unordered_map<std::string, std::string> instances;
    std::unordered_map<std::string, std::vector<model::NodeId>> node_sets;
    std::unordered_map<std::string, std::vector<model::ElementId>> element_sets;
    std::unordered_set<model::NodeId> node_ids;
    std::unordered_set<model::ElementId> element_ids;
    std::optional<AbaqusElementType> element_type;
    std::string only_part;

    for (const KeywordBlock &block : blocks)
    {
        if (block.keyword == "PART")
        {
            const std::string name = require_parameter(block, "NAME");
            parts.insert(uppercase_copy(name));
            only_part = name;
        }
        else if (block.keyword == "INSTANCE")
        {
            const std::string name = require_parameter(block, "NAME");
            const std::string part = require_parameter(block, "PART");
            if (!block.rows.empty())
            {
                throw AbaqusParseError(
                    "Transformed ABAQUS instances are not supported by this import profile.");
            }
            instances.emplace(uppercase_copy(name), part);
        }
    }
    if (parts.size() > 1 || instances.size() > 1)
    {
        throw AbaqusParseError(
            "This ABAQUS/CAE import profile supports one part and one instance per model.");
    }
    if (!instances.empty() && !parts.contains(uppercase_copy(instances.begin()->second)))
    {
        throw AbaqusParseError("ABAQUS instance references an unknown part.");
    }

    for (const KeywordBlock &block : blocks)
    {
        if (block.keyword == "NODE")
        {
            for (const DataRow &row : block.rows)
            {
                if (row.fields.size() < 3 || row.fields.size() > 4)
                {
                    throw AbaqusParseError("Expected node ID and coordinates on line " +
                                           std::to_string(row.line_number) + ".");
                }
                const model::NodeId id =
                    detail::parse_number<model::NodeId>(row.fields[0], row.line_number, "node ID");
                if (!node_ids.insert(id).second)
                {
                    throw AbaqusParseError("Duplicate ABAQUS node ID on line " +
                                           std::to_string(row.line_number) + ".");
                }
                std::array<double, 3> coordinates{};
                for (std::size_t coordinate = 1; coordinate < row.fields.size(); ++coordinate)
                {
                    coordinates[coordinate - 1] = detail::parse_number<double>(
                        row.fields[coordinate], row.line_number, "node coordinate");
                    if (!std::isfinite(coordinates[coordinate - 1]))
                    {
                        throw AbaqusParseError("Node coordinate must be finite on line " +
                                               std::to_string(row.line_number) + ".");
                    }
                }
                model.nodes.push_back({id, coordinates});
            }
        }
        else if (block.keyword == "ELEMENT")
        {
            const AbaqusElementType current_type =
                parse_element_type(require_parameter(block, "TYPE"), block.line_number);
            if (element_type.has_value() && *element_type != current_type)
            {
                throw AbaqusParseError("ABAQUS model cannot mix element types in this analysis.");
            }
            element_type = current_type;
            const std::size_t node_count = expected_node_count(current_type);
            std::vector<model::ElementId> block_element_ids;
            for (const DataRow &row : block.rows)
            {
                if (row.fields.size() != node_count + 1)
                {
                    throw AbaqusParseError("Incorrect element connectivity on line " +
                                           std::to_string(row.line_number) + ".");
                }
                const model::ElementId id = detail::parse_number<model::ElementId>(
                    row.fields[0], row.line_number, "element ID");
                if (!element_ids.insert(id).second)
                {
                    throw AbaqusParseError("Duplicate ABAQUS element ID on line " +
                                           std::to_string(row.line_number) + ".");
                }
                AbaqusImportedElement element{id, current_type, {}, {}, 1.0};
                element.node_ids.reserve(node_count);
                for (std::size_t node = 0; node < node_count; ++node)
                {
                    element.node_ids.push_back(detail::parse_number<model::NodeId>(
                        row.fields[node + 1], row.line_number, "element node ID"));
                }
                model.elements.push_back(std::move(element));
                block_element_ids.push_back(id);
            }
            const auto inline_set = block.parameters.find("ELSET");
            if (inline_set != block.parameters.end())
            {
                append_unique(element_sets, block_scope_key(block, inline_set->second),
                              block_element_ids, block.line_number);
            }
        }
        else if (block.keyword == "NSET")
        {
            const std::string name = require_parameter(block, "NSET");
            append_unique(node_sets, block_scope_key(block, name),
                          parse_set_ids<model::NodeId>(block, "node-set node ID"),
                          block.line_number);
        }
        else if (block.keyword == "ELSET")
        {
            const std::string name = require_parameter(block, "ELSET");
            append_unique(element_sets, block_scope_key(block, name),
                          parse_set_ids<model::ElementId>(block, "element-set element ID"),
                          block.line_number);
        }
    }

    if (model.nodes.empty())
    {
        throw AbaqusParseError("ABAQUS input does not contain node data.");
    }
    if (model.elements.empty() || !element_type.has_value())
    {
        throw AbaqusParseError("ABAQUS input does not contain supported element data.");
    }
    model.element_type = *element_type;
    for (const AbaqusImportedElement &element : model.elements)
    {
        for (const model::NodeId node_id : element.node_ids)
        {
            if (!node_ids.contains(node_id))
            {
                throw AbaqusParseError("ABAQUS element " + std::to_string(element.id) +
                                       " references unknown node " + std::to_string(node_id) + ".");
            }
        }
    }

    const auto materials = parse_abaqus_isotropic_materials(input_text);
    model.material_count = materials.size();
    std::unordered_set<std::string> material_names;
    for (const auto &material : materials)
    {
        material_names.insert(uppercase_copy(material.name));
    }

    std::unordered_map<model::ElementId, std::size_t> element_indices;
    for (std::size_t index = 0; index < model.elements.size(); ++index)
    {
        element_indices.emplace(model.elements[index].id, index);
    }
    std::unordered_set<model::ElementId> sectioned_elements;
    for (const KeywordBlock &block : blocks)
    {
        if (block.keyword != "SOLID SECTION")
        {
            continue;
        }
        const std::string set_name = require_parameter(block, "ELSET");
        const std::string material_name = require_parameter(block, "MATERIAL");
        if (!material_names.contains(uppercase_copy(material_name)))
        {
            throw AbaqusParseError("ABAQUS solid section references unknown material '" +
                                   material_name + "'.");
        }
        double thickness = 1.0;
        if (!block.rows.empty() && !block.rows.front().fields.empty() &&
            !block.rows.front().fields.front().empty())
        {
            thickness = detail::parse_number<double>(block.rows.front().fields.front(),
                                                     block.rows.front().line_number,
                                                     "solid-section thickness");
            if (!std::isfinite(thickness) || thickness <= 0.0)
            {
                throw AbaqusParseError("Solid-section thickness must be positive and finite.");
            }
        }
        const auto &ids = resolve_set(element_sets, set_name, block.part_name, block.in_assembly);
        for (const model::ElementId id : ids)
        {
            const auto element = element_indices.find(id);
            if (element == element_indices.end())
            {
                throw AbaqusParseError("ABAQUS solid section references unknown element " +
                                       std::to_string(id) + ".");
            }
            if (!sectioned_elements.insert(id).second)
            {
                throw AbaqusParseError("ABAQUS element has more than one solid section.");
            }
            model.elements[element->second].material_name = material_name;
            model.elements[element->second].section_thickness = thickness;
        }
    }
    if (sectioned_elements.size() != model.elements.size())
    {
        throw AbaqusParseError("Every ABAQUS element must belong to one solid section.");
    }

    const std::size_t dimension = abaqus_element_dimension(model.element_type);
    std::unordered_set<std::size_t> constrained_dofs;
    const auto resolve_nodes = [&](const std::string &target, const DataRow &row) {
        return resolve_numeric_or_set<model::NodeId>(target, node_sets, only_part, true,
                                                     row.line_number, "boundary node ID");
    };
    for (const KeywordBlock &block : blocks)
    {
        if (block.keyword == "BOUNDARY")
        {
            for (const DataRow &row : block.rows)
            {
                if (row.fields.size() < 2 || row.fields.size() > 4)
                {
                    throw AbaqusParseError("Invalid ABAQUS boundary data on line " +
                                           std::to_string(row.line_number) + ".");
                }
                const std::size_t first = detail::parse_number<std::size_t>(
                    row.fields[1], row.line_number, "boundary component");
                const std::size_t last =
                    row.fields.size() >= 3 && !row.fields[2].empty()
                        ? detail::parse_number<std::size_t>(row.fields[2], row.line_number,
                                                            "boundary component")
                        : first;
                const double value = row.fields.size() == 4 && !row.fields[3].empty()
                                         ? detail::parse_number<double>(
                                               row.fields[3], row.line_number, "boundary value")
                                         : 0.0;
                if (first == 0 || first > last || last > dimension || !std::isfinite(value))
                {
                    throw AbaqusParseError("Invalid ABAQUS boundary component range on line " +
                                           std::to_string(row.line_number) + ".");
                }
                for (const model::NodeId id : resolve_nodes(row.fields[0], row))
                {
                    if (!node_ids.contains(id))
                    {
                        throw AbaqusParseError("ABAQUS boundary references unknown node " +
                                               std::to_string(id) + ".");
                    }
                    for (std::size_t component = first; component <= last; ++component)
                    {
                        const std::size_t key = (id - 1) * dimension + component;
                        if (!constrained_dofs.insert(key).second)
                        {
                            throw AbaqusParseError(
                                "ABAQUS boundary conditions constrain a degree of freedom twice.");
                        }
                        model.prescribed_displacements.push_back({id, component, value});
                    }
                }
            }
        }
        else if (block.keyword == "CLOAD")
        {
            for (const DataRow &row : block.rows)
            {
                if (row.fields.size() != 3)
                {
                    throw AbaqusParseError("Invalid ABAQUS concentrated load on line " +
                                           std::to_string(row.line_number) + ".");
                }
                const std::size_t component = detail::parse_number<std::size_t>(
                    row.fields[1], row.line_number, "load component");
                const double magnitude =
                    detail::parse_number<double>(row.fields[2], row.line_number, "load magnitude");
                if (component == 0 || component > dimension || !std::isfinite(magnitude))
                {
                    throw AbaqusParseError("Invalid ABAQUS concentrated load on line " +
                                           std::to_string(row.line_number) + ".");
                }
                for (const model::NodeId id : resolve_nodes(row.fields[0], row))
                {
                    if (!node_ids.contains(id))
                    {
                        throw AbaqusParseError("ABAQUS load references unknown node " +
                                               std::to_string(id) + ".");
                    }
                    model.point_loads.push_back({id, component, magnitude});
                }
            }
        }
    }

    std::unordered_map<std::string, std::vector<SurfaceEntry>> surfaces;
    for (const KeywordBlock &block : blocks)
    {
        if (block.keyword != "SURFACE")
        {
            continue;
        }
        const auto surface_type = block.parameters.find("TYPE");
        if (surface_type == block.parameters.end() ||
            !detail::equals_case_insensitive(surface_type->second, "ELEMENT"))
        {
            throw AbaqusParseError("Only element-based ABAQUS surfaces are supported.");
        }
        const std::string name = require_parameter(block, "NAME");
        std::vector<SurfaceEntry> &entries = surfaces[block_scope_key(block, name)];
        for (const DataRow &row : block.rows)
        {
            if (row.fields.size() != 2)
            {
                throw AbaqusParseError("Invalid ABAQUS surface data on line " +
                                       std::to_string(row.line_number) + ".");
            }
            entries.push_back({row.fields[0], parse_face(row.fields[1], row.line_number),
                               block.part_name, block.in_assembly});
        }
    }

    const auto add_pressure = [&](const model::ElementId id, const std::size_t face,
                                  const double magnitude) {
        if (!element_ids.contains(id))
        {
            throw AbaqusParseError("ABAQUS pressure references unknown element " +
                                   std::to_string(id) + ".");
        }
        const std::size_t maximum_face = expected_node_count(model.element_type) == 3        ? 3
                                         : abaqus_element_dimension(model.element_type) == 2 ? 4
                                         : model.element_type == AbaqusElementType::c3d4     ? 4
                                                                                             : 6;
        if (face == 0 || face > maximum_face)
        {
            throw AbaqusParseError("Pressure face is invalid for the ABAQUS element type.");
        }
        model.pressure_loads.push_back({id, face, magnitude});
    };

    for (const KeywordBlock &block : blocks)
    {
        if (block.keyword == "DLOAD")
        {
            for (const DataRow &row : block.rows)
            {
                if (row.fields.size() != 3)
                {
                    throw AbaqusParseError("Invalid ABAQUS *DLOAD data on line " +
                                           std::to_string(row.line_number) + ".");
                }
                const std::size_t face = parse_face(row.fields[1], row.line_number);
                const double magnitude = detail::parse_number<double>(
                    row.fields[2], row.line_number, "pressure magnitude");
                for (const model::ElementId id : resolve_numeric_or_set<model::ElementId>(
                         row.fields[0], element_sets, only_part, true, row.line_number,
                         "pressure element ID"))
                {
                    add_pressure(id, face, magnitude);
                }
            }
        }
        else if (block.keyword == "DSLOAD")
        {
            for (const DataRow &row : block.rows)
            {
                if (row.fields.size() != 3 || !detail::equals_case_insensitive(row.fields[1], "P"))
                {
                    throw AbaqusParseError("Only uniform pressure ABAQUS *DSLOAD data is supported "
                                           "on line " +
                                           std::to_string(row.line_number) + ".");
                }
                const double magnitude = detail::parse_number<double>(
                    row.fields[2], row.line_number, "surface pressure magnitude");
                const std::string assembly_key = scope_key("ASSEMBLY", {}, row.fields[0]);
                const std::string global_key = scope_key("GLOBAL", {}, row.fields[0]);
                auto surface = surfaces.find(assembly_key);
                if (surface == surfaces.end())
                {
                    surface = surfaces.find(global_key);
                }
                if (surface == surfaces.end())
                {
                    throw AbaqusParseError("ABAQUS *DSLOAD references unknown surface '" +
                                           row.fields[0] + "'.");
                }
                for (const SurfaceEntry &entry : surface->second)
                {
                    for (const model::ElementId id :
                         resolve_set(element_sets, entry.element_set, only_part, entry.in_assembly))
                    {
                        add_pressure(id, entry.face, magnitude);
                    }
                }
            }
        }
    }
    return model;
}
} // namespace finelemethod::input
