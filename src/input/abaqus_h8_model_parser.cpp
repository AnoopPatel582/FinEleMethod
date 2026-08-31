#include "finelemethod/input/abaqus_h8_model_parser.hpp"

#include "abaqus_parser_utilities.hpp"

#include "finelemethod/input/abaqus_boundary_parser.hpp"
#include "finelemethod/input/abaqus_cload_parser.hpp"
#include "finelemethod/input/abaqus_element_parser.hpp"
#include "finelemethod/input/abaqus_material_parser.hpp"
#include "finelemethod/input/abaqus_node_parser.hpp"
#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/h8_element.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

namespace finelemethod::input
{
namespace
{
struct SolidSectionAssignment
{
    std::string element_set;
    std::string material_name;
};

std::string uppercase_copy(const std::string_view text)
{
    std::string uppercase(text);
    for (char &character : uppercase)
    {
        character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    }
    return uppercase;
}

std::vector<SolidSectionAssignment> parse_solid_section_assignments(
    const std::string_view input_text)
{
    std::vector<SolidSectionAssignment> assignments;
    std::unordered_set<std::string> assigned_element_sets;
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

        if (line.starts_with('*') && !line.starts_with("**"))
        {
            const std::size_t comma = line.find(',');
            const std::string_view keyword = detail::trim(
                line.substr(1, comma == std::string_view::npos ? line.size() - 1 : comma - 1));
            if (detail::equals_case_insensitive(keyword, "SOLID SECTION"))
            {
                SolidSectionAssignment assignment;
                const auto fields = detail::split_fields(line);
                for (std::size_t field_index = 1; field_index < fields.size(); ++field_index)
                {
                    const std::size_t equals = fields[field_index].find('=');
                    if (equals == std::string_view::npos)
                    {
                        continue;
                    }
                    const std::string_view name =
                        detail::trim(fields[field_index].substr(0, equals));
                    const std::string_view value =
                        detail::trim(fields[field_index].substr(equals + 1));
                    if (detail::equals_case_insensitive(name, "ELSET"))
                    {
                        assignment.element_set = std::string(value);
                    }
                    else if (detail::equals_case_insensitive(name, "MATERIAL"))
                    {
                        assignment.material_name = std::string(value);
                    }
                }
                if (assignment.element_set.empty() || assignment.material_name.empty())
                {
                    throw AbaqusParseError(
                        "ABAQUS *SOLID SECTION requires ELSET and MATERIAL on line " +
                        std::to_string(line_number) + ".");
                }
                if (!assigned_element_sets.insert(uppercase_copy(assignment.element_set)).second)
                {
                    throw AbaqusParseError("Duplicate solid section for element set on line " +
                                           std::to_string(line_number) + ".");
                }
                assignments.push_back(std::move(assignment));
            }
        }

        if (line_end == std::string_view::npos)
        {
            break;
        }
        line_start = line_end + 1;
    }

    if (assignments.empty())
    {
        throw AbaqusParseError("ABAQUS input does not contain a *SOLID SECTION definition.");
    }
    return assignments;
}
} // namespace

AbaqusH8Model parse_abaqus_h8_model(const std::string_view input_text)
{
    AbaqusH8Model model;
    model.nodes = parse_abaqus_nodes(input_text);

    const auto parsed_materials = parse_abaqus_isotropic_materials(input_text);
    std::unordered_map<std::string, model::MaterialId> material_ids_by_name;
    for (std::size_t index = 0; index < parsed_materials.size(); ++index)
    {
        const model::MaterialId material_id = index + 1;
        const AbaqusIsotropicMaterial &material = parsed_materials[index];
        model.materials.add(model::IsotropicElasticMaterial(material_id, material.youngs_modulus,
                                                            material.poisson_ratio));
        material_ids_by_name.emplace(uppercase_copy(material.name), material_id);
    }

    std::unordered_map<std::string, model::MaterialId> material_ids_by_element_set;
    for (const SolidSectionAssignment &section : parse_solid_section_assignments(input_text))
    {
        const auto material = material_ids_by_name.find(uppercase_copy(section.material_name));
        if (material == material_ids_by_name.end())
        {
            throw AbaqusParseError("ABAQUS solid section for element set '" + section.element_set +
                                   "' references unknown material '" + section.material_name +
                                   "'.");
        }
        material_ids_by_element_set.emplace(uppercase_copy(section.element_set), material->second);
    }

    for (const AbaqusH8Element &element : parse_abaqus_h8_elements(input_text))
    {
        if (element.element_set.empty())
        {
            throw AbaqusParseError("ABAQUS H8 element " + std::to_string(element.id) +
                                   " does not belong to an element set.");
        }
        const auto material_id =
            material_ids_by_element_set.find(uppercase_copy(element.element_set));
        if (material_id == material_ids_by_element_set.end())
        {
            throw AbaqusParseError("ABAQUS H8 element " + std::to_string(element.id) +
                                   " references element set '" + element.element_set +
                                   "' without a solid section.");
        }
        for (const model::NodeId node_id : element.node_ids)
        {
            if (!model.nodes.contains(node_id))
            {
                throw AbaqusParseError("ABAQUS H8 element " + std::to_string(element.id) +
                                       " references unknown node " + std::to_string(node_id) + ".");
            }
        }
        model.elements.add(model::H8Element(element.id, element.node_ids, material_id->second));
    }

    const auto parsed_displacements =
        parse_abaqus_nodal_displacements(input_text, model::SpatialDimension::three_dimensional);
    std::vector<AbaqusConcentratedLoad> parsed_loads;
    if (detail::contains_keyword(input_text, "CLOAD"))
    {
        parsed_loads =
            parse_abaqus_concentrated_loads(input_text, model::SpatialDimension::three_dimensional);
    }
    const bool uses_node_sets =
        std::any_of(parsed_displacements.begin(), parsed_displacements.end(),
                    [](const AbaqusNodalDisplacement &displacement) {
                        return std::holds_alternative<std::string>(displacement.target);
                    }) ||
        std::any_of(parsed_loads.begin(), parsed_loads.end(),
                    [](const AbaqusConcentratedLoad &load) {
                        return std::holds_alternative<std::string>(load.target);
                    });
    std::unordered_map<std::string, const AbaqusNodeSet *> node_sets_by_name;
    if (uses_node_sets)
    {
        model.node_sets = parse_abaqus_node_sets(input_text);
        for (const AbaqusNodeSet &node_set : model.node_sets)
        {
            node_sets_by_name.emplace(uppercase_copy(node_set.name), &node_set);
        }
    }

    const model::DofMap dof_map(model.nodes, model::SpatialDimension::three_dimensional);
    std::unordered_set<std::size_t> constrained_dofs;
    const auto add_displacement = [&](const model::NodeId node_id,
                                      const model::DisplacementComponent component,
                                      const double value) {
        if (!model.nodes.contains(node_id))
        {
            throw AbaqusParseError("ABAQUS boundary condition references unknown node " +
                                   std::to_string(node_id) + ".");
        }
        const std::size_t global_dof = dof_map.global_index(node_id, component);
        if (!constrained_dofs.insert(global_dof).second)
        {
            throw AbaqusParseError("ABAQUS boundary conditions constrain the same node degree of "
                                   "freedom more than once.");
        }
        model.prescribed_displacements.push_back(solver::PrescribedDisplacement{global_dof, value});
    };

    for (const AbaqusNodalDisplacement &displacement : parsed_displacements)
    {
        if (const auto *node_id = std::get_if<model::NodeId>(&displacement.target))
        {
            add_displacement(*node_id, displacement.component, displacement.value);
            continue;
        }
        const std::string &set_name = std::get<std::string>(displacement.target);
        const auto node_set = node_sets_by_name.find(uppercase_copy(set_name));
        if (node_set == node_sets_by_name.end())
        {
            throw AbaqusParseError("ABAQUS boundary condition references unknown node set '" +
                                   set_name + "'.");
        }
        for (const model::NodeId node_id : node_set->second->node_ids)
        {
            add_displacement(node_id, displacement.component, displacement.value);
        }
    }

    const auto add_point_load = [&](const model::NodeId node_id,
                                    const model::DisplacementComponent component,
                                    const double magnitude) {
        if (!model.nodes.contains(node_id))
        {
            throw AbaqusParseError("ABAQUS concentrated load references unknown node " +
                                   std::to_string(node_id) + ".");
        }
        model.point_loads.emplace_back(node_id, component, magnitude);
    };
    for (const AbaqusConcentratedLoad &load : parsed_loads)
    {
        if (const auto *node_id = std::get_if<model::NodeId>(&load.target))
        {
            add_point_load(*node_id, load.component, load.magnitude);
            continue;
        }
        const std::string &set_name = std::get<std::string>(load.target);
        const auto node_set = node_sets_by_name.find(uppercase_copy(set_name));
        if (node_set == node_sets_by_name.end())
        {
            throw AbaqusParseError("ABAQUS concentrated load references unknown node set '" +
                                   set_name + "'.");
        }
        for (const model::NodeId node_id : node_set->second->node_ids)
        {
            add_point_load(node_id, load.component, load.magnitude);
        }
    }

    return model;
}
} // namespace finelemethod::input
