#include "finelemethod/input/abaqus_q4_model_parser.hpp"

#include "finelemethod/input/abaqus_boundary_parser.hpp"
#include "finelemethod/input/abaqus_element_parser.hpp"
#include "finelemethod/input/abaqus_material_parser.hpp"
#include "finelemethod/input/abaqus_node_parser.hpp"
#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/input/abaqus_solid_section_parser.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"
#include "finelemethod/model/q4_element.hpp"

#include <cctype>
#include <string>
#include <unordered_map>
#include <utility>

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
    }

    const model::DofMap dof_map(model.nodes, model::SpatialDimension::two_dimensional);
    const auto parsed_displacements = parse_abaqus_nodal_displacements(input_text);
    model.prescribed_displacements.reserve(parsed_displacements.size());
    for (const AbaqusNodalDisplacement &displacement : parsed_displacements)
    {
        if (!model.nodes.contains(displacement.node_id))
        {
            throw AbaqusParseError("ABAQUS boundary condition references unknown node " +
                                   std::to_string(displacement.node_id) + ".");
        }
        model.prescribed_displacements.push_back(solver::PrescribedDisplacement{
            dof_map.global_index(displacement.node_id, displacement.component),
            displacement.value});
    }

    return model;
}
} // namespace finelemethod::input
