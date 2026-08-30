#include "finelemethod/input/abaqus_q4_model_parser.hpp"

#include "abaqus_parser_utilities.hpp"

#include "finelemethod/input/abaqus_boundary_parser.hpp"
#include "finelemethod/input/abaqus_cload_parser.hpp"
#include "finelemethod/input/abaqus_dload_parser.hpp"
#include "finelemethod/input/abaqus_element_parser.hpp"
#include "finelemethod/input/abaqus_material_parser.hpp"
#include "finelemethod/input/abaqus_node_parser.hpp"
#include "finelemethod/input/abaqus_node_set_parser.hpp"
#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/input/abaqus_solid_section_parser.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"
#include "finelemethod/model/q4_element.hpp"

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
std::string uppercase_copy(const std::string_view text)
{
    std::string uppercase(text);
    for (char &character : uppercase)
    {
        character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    }
    return uppercase;
}
} // namespace

AbaqusQ4Model parse_abaqus_q4_model(const std::string_view input_text)
{
    AbaqusQ4Model model;
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

    const auto parsed_sections = parse_abaqus_solid_sections(input_text);
    std::unordered_map<std::string, const AbaqusSolidSection *> sections_by_element_set;
    for (const AbaqusSolidSection &section : parsed_sections)
    {
        const auto material = material_ids_by_name.find(uppercase_copy(section.material_name));
        if (material == material_ids_by_name.end())
        {
            throw AbaqusParseError("ABAQUS solid section for element set '" + section.element_set +
                                   "' references unknown material '" + section.material_name +
                                   "'.");
        }
        sections_by_element_set.emplace(uppercase_copy(section.element_set), &section);
    }

    const auto parsed_elements = parse_abaqus_q4_elements(input_text);
    std::unordered_map<std::string, std::vector<model::ElementId>> element_ids_by_set;
    for (const AbaqusQ4Element &element : parsed_elements)
    {
        if (element.element_set.empty())
        {
            throw AbaqusParseError("ABAQUS Q4 element " + std::to_string(element.id) +
                                   " does not belong to an element set.");
        }

        const auto section = sections_by_element_set.find(uppercase_copy(element.element_set));
        if (section == sections_by_element_set.end())
        {
            throw AbaqusParseError("ABAQUS Q4 element " + std::to_string(element.id) +
                                   " references element set '" + element.element_set +
                                   "' without a solid section.");
        }

        for (const model::NodeId node_id : element.node_ids)
        {
            if (!model.nodes.contains(node_id))
            {
                throw AbaqusParseError("ABAQUS Q4 element " + std::to_string(element.id) +
                                       " references unknown node " + std::to_string(node_id) + ".");
            }
        }

        const AbaqusSolidSection &solid_section = *section->second;
        const model::MaterialId material_id =
            material_ids_by_name.at(uppercase_copy(solid_section.material_name));
        model.elements.add(
            model::Q4Element(element.id, element.node_ids, material_id, solid_section.thickness));
        element_ids_by_set[uppercase_copy(element.element_set)].push_back(element.id);
    }

    const auto parsed_displacements = parse_abaqus_nodal_displacements(input_text);
    const auto parsed_loads = parse_abaqus_concentrated_loads(input_text);
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

    const model::DofMap dof_map(model.nodes, model::SpatialDimension::two_dimensional);
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

    if (detail::contains_keyword(input_text, "DLOAD"))
    {
        const auto parsed_pressures = parse_abaqus_q4_edge_pressures(input_text);
        const auto add_pressure = [&](const model::ElementId element_id, const model::Q4Edge edge,
                                      const double pressure) {
            if (!model.elements.contains(element_id))
            {
                throw AbaqusParseError("ABAQUS distributed load references unknown Q4 element " +
                                       std::to_string(element_id) + ".");
            }
            model.pressure_loads.emplace_back(element_id, edge, pressure);
        };

        for (const AbaqusQ4EdgePressure &pressure : parsed_pressures)
        {
            if (const auto *element_id = std::get_if<model::ElementId>(&pressure.target))
            {
                add_pressure(*element_id, pressure.edge, pressure.pressure);
                continue;
            }

            const std::string &set_name = std::get<std::string>(pressure.target);
            const auto element_set = element_ids_by_set.find(uppercase_copy(set_name));
            if (element_set == element_ids_by_set.end())
            {
                throw AbaqusParseError(
                    "ABAQUS distributed load references unknown Q4 element set '" + set_name +
                    "'.");
            }
            for (const model::ElementId element_id : element_set->second)
            {
                add_pressure(element_id, pressure.edge, pressure.pressure);
            }
        }
    }

    return model;
}
} // namespace finelemethod::input
