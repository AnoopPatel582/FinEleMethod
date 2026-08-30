#include "finelemethod/examples/q4_tension_example.hpp"

#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/point_load.hpp"
#include "finelemethod/model/q4_edge_pressure_load.hpp"
#include "finelemethod/model/q4_element.hpp"
#include "finelemethod/model/q4_element_collection.hpp"
#include "finelemethod/output/q4_analysis_vtu.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"
#include "finelemethod/solver/q4_plane_stress_analysis.hpp"

#include <array>

namespace finelemethod::examples
{
void write_q4_tension_example(const std::filesystem::path &output_path)
{
    model::NodeCollection nodes;
    nodes.add(model::Node(1, 0.0, 0.0));
    nodes.add(model::Node(2, 1.0, 0.0));
    nodes.add(model::Node(3, 1.0, 1.0));
    nodes.add(model::Node(4, 0.0, 1.0));
    const model::DofMap dof_map(nodes, model::SpatialDimension::two_dimensional);

    model::MaterialCollection materials;
    materials.add(model::IsotropicElasticMaterial(1, 1000.0, 0.25));

    model::Q4ElementCollection elements;
    elements.add(model::Q4Element(1, model::Q4NodeIds{{1, 2, 3, 4}}, 1, 1.0));

    const std::array point_loads{
        model::PointLoad(2, model::DisplacementComponent::x, 5.0),
        model::PointLoad(3, model::DisplacementComponent::x, 5.0),
    };
    const std::array prescribed_displacements{
        solver::PrescribedDisplacement{dof_map.global_index(1, model::DisplacementComponent::x),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(1, model::DisplacementComponent::y),
                                       0.0},
        solver::PrescribedDisplacement{dof_map.global_index(4, model::DisplacementComponent::x),
                                       0.0},
    };

    const std::array<model::Q4EdgePressureLoad, 0> pressure_loads{};
    const auto result = solver::solve_q4_plane_stress_model(
        elements, nodes, materials, dof_map, point_loads, pressure_loads, prescribed_displacements);
    output::write_q4_analysis_vtu(output_path, nodes, elements, dof_map, result);
}
} // namespace finelemethod::examples
