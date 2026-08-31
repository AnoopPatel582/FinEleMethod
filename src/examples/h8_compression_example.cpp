#include "finelemethod/examples/h8_compression_example.hpp"

#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/h8_element.hpp"
#include "finelemethod/model/h8_element_collection.hpp"
#include "finelemethod/model/h8_face_pressure_load.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/point_load.hpp"
#include "finelemethod/output/h8_analysis_vtu.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"
#include "finelemethod/solver/h8_analysis.hpp"

#include <array>

namespace finelemethod::examples
{
void write_h8_compression_example(const std::filesystem::path &output_path)
{
    model::NodeCollection nodes;
    nodes.add(model::Node(1, 0.0, 0.0, 0.0));
    nodes.add(model::Node(2, 1.0, 0.0, 0.0));
    nodes.add(model::Node(3, 1.0, 1.0, 0.0));
    nodes.add(model::Node(4, 0.0, 1.0, 0.0));
    nodes.add(model::Node(5, 0.0, 0.0, 1.0));
    nodes.add(model::Node(6, 1.0, 0.0, 1.0));
    nodes.add(model::Node(7, 1.0, 1.0, 1.0));
    nodes.add(model::Node(8, 0.0, 1.0, 1.0));
    const model::DofMap dof_map(nodes, model::SpatialDimension::three_dimensional);

    model::MaterialCollection materials;
    materials.add(model::IsotropicElasticMaterial(1, 1000.0, 0.25));
    model::H8ElementCollection elements;
    elements.add(model::H8Element(1, model::H8NodeIds{{1, 2, 3, 4, 5, 6, 7, 8}}, 1));

    const std::array point_loads{
        model::PointLoad(5, model::DisplacementComponent::z, -2.5),
        model::PointLoad(6, model::DisplacementComponent::z, -2.5),
        model::PointLoad(7, model::DisplacementComponent::z, -2.5),
        model::PointLoad(8, model::DisplacementComponent::z, -2.5),
    };
    const std::array prescribed_displacements{
        solver::PrescribedDisplacement{dof_map.global_index(1, model::DisplacementComponent::z),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(2, model::DisplacementComponent::z),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(3, model::DisplacementComponent::z),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(4, model::DisplacementComponent::z),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(1, model::DisplacementComponent::x),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(4, model::DisplacementComponent::x),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(5, model::DisplacementComponent::x),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(8, model::DisplacementComponent::x),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(1, model::DisplacementComponent::y),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(2, model::DisplacementComponent::y),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(5, model::DisplacementComponent::y),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(6, model::DisplacementComponent::y),
                                       0.0},
    };
    const std::array<model::H8FacePressureLoad, 0> pressure_loads{};

    const auto result = solver::solve_h8_model(elements, nodes, materials, dof_map, point_loads,
                                               pressure_loads, prescribed_displacements);
    output::write_h8_analysis_vtu(output_path, nodes, elements, dof_map, result);
}
} // namespace finelemethod::examples
