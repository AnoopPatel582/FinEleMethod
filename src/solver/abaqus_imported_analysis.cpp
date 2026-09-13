#include "finelemethod/solver/abaqus_imported_analysis.hpp"

#include "finelemethod/assembly/coo_assembly.hpp"
#include "finelemethod/elements/h8_face_pressure.hpp"
#include "finelemethod/elements/h8_reduced_stiffness.hpp"
#include "finelemethod/elements/h8_strain_displacement.hpp"
#include "finelemethod/elements/q4_edge_pressure.hpp"
#include "finelemethod/elements/q4_reduced_stiffness.hpp"
#include "finelemethod/elements/q4_strain_displacement.hpp"
#include "finelemethod/elements/t3.hpp"
#include "finelemethod/elements/t4.hpp"
#include "finelemethod/input/abaqus_material_parser.hpp"
#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/mechanics/constitutive_matrix.hpp"
#include "finelemethod/mechanics/plane_stress_measures.hpp"
#include "finelemethod/mechanics/solid_stress_measures.hpp"
#include "finelemethod/model/h8_face_pressure_load.hpp"
#include "finelemethod/model/q4_edge_pressure_load.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"
#include "finelemethod/solver/sparse_static_solver.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace finelemethod::solver
{
namespace
{
struct AnalysisContext
{
    std::size_t dimension{};
    std::unordered_map<model::NodeId, std::size_t> node_indices;
    std::vector<model::IsotropicElasticMaterial> materials;
    std::unordered_map<std::string, std::size_t> material_indices;
    std::unordered_map<model::ElementId, std::size_t> element_indices;
};

std::string uppercase_copy(const std::string_view text)
{
    std::string result(text);
    std::ranges::transform(result, result.begin(), [](const unsigned char character) {
        return static_cast<char>(std::toupper(character));
    });
    return result;
}

bool is_supported_solver_type(const input::AbaqusElementType type)
{
    return type == input::AbaqusElementType::cps3 || type == input::AbaqusElementType::cps4r ||
           type == input::AbaqusElementType::c3d4 || type == input::AbaqusElementType::c3d8r;
}

AnalysisContext build_context(const input::AbaqusImportedModel &model,
                              const std::string_view input_text)
{
    AnalysisContext context;
    context.dimension = input::abaqus_element_dimension(model.element_type);
    context.node_indices.reserve(model.nodes.size());
    for (std::size_t index = 0; index < model.nodes.size(); ++index)
    {
        context.node_indices.emplace(model.nodes[index].id, index);
    }
    context.element_indices.reserve(model.elements.size());
    for (std::size_t index = 0; index < model.elements.size(); ++index)
    {
        context.element_indices.emplace(model.elements[index].id, index);
    }

    const auto imported_materials = input::parse_abaqus_isotropic_materials(input_text);
    context.materials.reserve(imported_materials.size());
    context.material_indices.reserve(imported_materials.size());
    for (std::size_t index = 0; index < imported_materials.size(); ++index)
    {
        context.material_indices.emplace(uppercase_copy(imported_materials[index].name), index);
        context.materials.emplace_back(index + 1, imported_materials[index].youngs_modulus,
                                       imported_materials[index].poisson_ratio);
    }
    return context;
}

const model::IsotropicElasticMaterial &element_material(const input::AbaqusImportedElement &element,
                                                        const AnalysisContext &context)
{
    const auto material = context.material_indices.find(uppercase_copy(element.material_name));
    if (material == context.material_indices.end())
    {
        throw input::AbaqusParseError("Element references an unresolved ABAQUS material.");
    }
    return context.materials[material->second];
}

std::vector<std::size_t> element_dofs(const input::AbaqusImportedElement &element,
                                      const AnalysisContext &context)
{
    std::vector<std::size_t> indices;
    indices.reserve(element.node_ids.size() * context.dimension);
    for (const model::NodeId node_id : element.node_ids)
    {
        const auto node = context.node_indices.find(node_id);
        if (node == context.node_indices.end())
        {
            throw input::AbaqusParseError("Element references an unresolved ABAQUS node.");
        }
        for (std::size_t component = 0; component < context.dimension; ++component)
        {
            indices.push_back(node->second * context.dimension + component);
        }
    }
    return indices;
}

elements::T3NodeCoordinates t3_coordinates(const input::AbaqusImportedElement &element,
                                           const input::AbaqusImportedModel &model,
                                           const AnalysisContext &context)
{
    elements::T3NodeCoordinates coordinates{};
    for (std::size_t index = 0; index < coordinates.size(); ++index)
    {
        const auto node = context.node_indices.at(element.node_ids[index]);
        coordinates[index] = {model.nodes[node].coordinates[0], model.nodes[node].coordinates[1]};
    }
    return coordinates;
}

elements::Q4NodeCoordinates q4_coordinates(const input::AbaqusImportedElement &element,
                                           const input::AbaqusImportedModel &model,
                                           const AnalysisContext &context)
{
    elements::Q4NodeCoordinates coordinates{};
    for (std::size_t index = 0; index < coordinates.size(); ++index)
    {
        const auto node = context.node_indices.at(element.node_ids[index]);
        coordinates[index] = {model.nodes[node].coordinates[0], model.nodes[node].coordinates[1]};
    }
    return coordinates;
}

elements::T4NodeCoordinates t4_coordinates(const input::AbaqusImportedElement &element,
                                           const input::AbaqusImportedModel &model,
                                           const AnalysisContext &context)
{
    elements::T4NodeCoordinates coordinates{};
    for (std::size_t index = 0; index < coordinates.size(); ++index)
    {
        coordinates[index] =
            model.nodes[context.node_indices.at(element.node_ids[index])].coordinates;
    }
    return coordinates;
}

elements::H8NodeCoordinates h8_coordinates(const input::AbaqusImportedElement &element,
                                           const input::AbaqusImportedModel &model,
                                           const AnalysisContext &context)
{
    elements::H8NodeCoordinates coordinates{};
    for (std::size_t index = 0; index < coordinates.size(); ++index)
    {
        coordinates[index] =
            model.nodes[context.node_indices.at(element.node_ids[index])].coordinates;
    }
    return coordinates;
}

math::DenseMatrix element_stiffness(const input::AbaqusImportedElement &element,
                                    const input::AbaqusImportedModel &model,
                                    const AnalysisContext &context)
{
    const auto &material = element_material(element, context);
    switch (element.type)
    {
    case input::AbaqusElementType::cps3:
        return elements::t3_plane_stress_stiffness_matrix(t3_coordinates(element, model, context),
                                                          material, element.section_thickness);
    case input::AbaqusElementType::cps4r:
        return elements::q4_reduced_plane_stress_stiffness_matrix(
            q4_coordinates(element, model, context), material, element.section_thickness);
    case input::AbaqusElementType::c3d4:
        return elements::t4_stiffness_matrix(t4_coordinates(element, model, context), material);
    case input::AbaqusElementType::c3d8r:
        return elements::h8_reduced_stiffness_matrix(h8_coordinates(element, model, context),
                                                     material);
    default:
        throw input::AbaqusParseError(
            "Canonical imported analysis received an unsupported element formulation.");
    }
}

void add_local_load(math::DenseVector &global_load, const math::DenseVector &local_load,
                    const std::vector<std::size_t> &dofs)
{
    if (local_load.size() != dofs.size())
    {
        throw std::logic_error("Element pressure load and DOF mapping sizes do not match.");
    }
    for (std::size_t index = 0; index < dofs.size(); ++index)
    {
        global_load[dofs[index]] += local_load[index];
    }
}

math::DenseVector pressure_load(const input::AbaqusImportedPressure &pressure,
                                const input::AbaqusImportedElement &element,
                                const input::AbaqusImportedModel &model,
                                const AnalysisContext &context)
{
    switch (element.type)
    {
    case input::AbaqusElementType::cps3:
        return elements::t3_uniform_edge_pressure_load(
            t3_coordinates(element, model, context),
            static_cast<elements::T3Edge>(pressure.face - 1), pressure.magnitude,
            element.section_thickness);
    case input::AbaqusElementType::cps4r:
        return elements::q4_uniform_edge_pressure_load(
            q4_coordinates(element, model, context), static_cast<model::Q4Edge>(pressure.face - 1),
            pressure.magnitude, element.section_thickness);
    case input::AbaqusElementType::c3d4:
        return elements::t4_uniform_face_pressure_load(
            t4_coordinates(element, model, context),
            static_cast<elements::T4Face>(pressure.face - 1), pressure.magnitude);
    case input::AbaqusElementType::c3d8r:
        return elements::h8_uniform_face_pressure_load(
            h8_coordinates(element, model, context), static_cast<model::H8Face>(pressure.face - 1),
            pressure.magnitude);
    default:
        throw input::AbaqusParseError("Pressure load uses an unsupported element formulation.");
    }
}

math::DenseVector local_displacements(const input::AbaqusImportedElement &element,
                                      const AnalysisContext &context,
                                      const math::DenseVector &global_displacements)
{
    const auto dofs = element_dofs(element, context);
    math::DenseVector local(dofs.size());
    for (std::size_t index = 0; index < dofs.size(); ++index)
    {
        local[index] = global_displacements[dofs[index]];
    }
    return local;
}

AbaqusImportedElementResult recover_result(const input::AbaqusImportedElement &element,
                                           const input::AbaqusImportedModel &model,
                                           const AnalysisContext &context,
                                           const math::DenseVector &global_displacements)
{
    const auto &material = element_material(element, context);
    const math::DenseVector local = local_displacements(element, context, global_displacements);
    math::DenseVector strain(context.dimension == 2 ? 3 : 6);
    if (element.type == input::AbaqusElementType::cps3)
    {
        strain = elements::t3_strain_displacement_matrix(t3_coordinates(element, model, context))
                     .matrix *
                 local;
    }
    else if (element.type == input::AbaqusElementType::cps4r)
    {
        strain = elements::q4_strain_displacement_matrix(q4_coordinates(element, model, context),
                                                         0.0, 0.0)
                     .matrix *
                 local;
    }
    else if (element.type == input::AbaqusElementType::c3d4)
    {
        strain = elements::t4_strain_displacement_matrix(t4_coordinates(element, model, context))
                     .matrix *
                 local;
    }
    else if (element.type == input::AbaqusElementType::c3d8r)
    {
        strain = elements::h8_strain_displacement_matrix(h8_coordinates(element, model, context),
                                                         0.0, 0.0, 0.0)
                     .matrix *
                 local;
    }
    else
    {
        throw input::AbaqusParseError("Result recovery uses an unsupported element formulation.");
    }

    AbaqusImportedElementResult result{.element_id = element.id};
    if (context.dimension == 2)
    {
        const math::DenseVector stress =
            mechanics::plane_stress_constitutive_matrix(material) * strain;
        result.strain = {strain[0], strain[1], 0.0, strain[2], 0.0, 0.0};
        result.stress = {stress[0], stress[1], 0.0, stress[2], 0.0, 0.0};
        const auto measures =
            mechanics::calculate_plane_stress_measures(stress[0], stress[1], stress[2]);
        result.von_mises = measures.von_mises;
        result.principal_stresses = measures.principal_stresses;
    }
    else
    {
        const math::DenseVector stress =
            mechanics::solid_isotropic_constitutive_matrix(material) * strain;
        for (std::size_t component = 0; component < 6; ++component)
        {
            result.strain[component] = strain[component];
            result.stress[component] = stress[component];
        }
        const auto measures = mechanics::calculate_solid_stress_measures(
            stress[0], stress[1], stress[2], stress[3], stress[4], stress[5]);
        result.von_mises = measures.von_mises;
        result.principal_stresses = measures.principal_stresses;
    }
    return result;
}
} // namespace

AbaqusImportedAnalysisSolution analyze_abaqus_imported_model(
    const std::string_view input_text, const ConjugateGradientOptions &solver_options)
{
    input::AbaqusImportedModel model = input::import_abaqus_cae_model(input_text);
    if (!is_supported_solver_type(model.element_type))
    {
        throw input::AbaqusParseError(
            "Canonical imported analysis supports CPS3, CPS4R, C3D4, and C3D8R elements.");
    }
    const AnalysisContext context = build_context(model, input_text);
    const std::size_t dof_count = model.nodes.size() * context.dimension;
    math::CooMatrix stiffness(dof_count, dof_count);
    for (const auto &element : model.elements)
    {
        const auto dofs = element_dofs(element, context);
        assembly::add_element_matrix_to_coo(stiffness, element_stiffness(element, model, context),
                                            dofs);
    }

    math::DenseVector load(dof_count);
    for (const auto &point_load : model.point_loads)
    {
        const std::size_t node = context.node_indices.at(point_load.node_id);
        load[node * context.dimension + point_load.component - 1] += point_load.magnitude;
    }
    for (const auto &pressure : model.pressure_loads)
    {
        const auto element_index = context.element_indices.at(pressure.element_id);
        const auto &element = model.elements[element_index];
        add_local_load(load, pressure_load(pressure, element, model, context),
                       element_dofs(element, context));
    }

    std::vector<PrescribedDisplacement> constraints;
    constraints.reserve(model.prescribed_displacements.size());
    for (const auto &constraint : model.prescribed_displacements)
    {
        constraints.push_back({context.node_indices.at(constraint.node_id) * context.dimension +
                                   constraint.component - 1,
                               constraint.value});
    }

    SparseStaticSolution static_solution =
        solve_sparse_static_system(stiffness, load, constraints, solver_options);
    std::vector<AbaqusImportedElementResult> results;
    results.reserve(model.elements.size());
    for (const auto &element : model.elements)
    {
        results.push_back(recover_result(element, model, context, static_solution.displacements));
    }

    return {
        std::move(model),
        std::move(static_solution.displacements),
        std::move(static_solution.reactions),
        std::move(results),
        static_solution.iterations,
        static_solution.residual_norm,
    };
}
} // namespace finelemethod::solver
