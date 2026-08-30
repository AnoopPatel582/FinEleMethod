#include "finelemethod/solver/h8_analysis.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>

namespace
{
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::H8Element;
using finelemethod::model::H8ElementCollection;
using finelemethod::model::H8NodeIds;
using finelemethod::model::IsotropicElasticMaterial;
using finelemethod::model::MaterialCollection;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::PointLoad;
using finelemethod::model::SpatialDimension;
using finelemethod::solver::PrescribedDisplacement;
using finelemethod::solver::solve_h8_model;

TEST(H8Analysis, ReproducesUniformUniaxialCompression)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0, 0.0));
    nodes.add(Node(2, 1.0, 0.0, 0.0));
    nodes.add(Node(3, 1.0, 1.0, 0.0));
    nodes.add(Node(4, 0.0, 1.0, 0.0));
    nodes.add(Node(5, 0.0, 0.0, 1.0));
    nodes.add(Node(6, 1.0, 0.0, 1.0));
    nodes.add(Node(7, 1.0, 1.0, 1.0));
    nodes.add(Node(8, 0.0, 1.0, 1.0));
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);

    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(1, 1000.0, 0.25));
    H8ElementCollection elements;
    elements.add(H8Element(1, H8NodeIds{{1, 2, 3, 4, 5, 6, 7, 8}}, 1));

    const std::array point_loads{
        PointLoad(5, DisplacementComponent::z, -2.5),
        PointLoad(6, DisplacementComponent::z, -2.5),
        PointLoad(7, DisplacementComponent::z, -2.5),
        PointLoad(8, DisplacementComponent::z, -2.5),
    };
    const std::array prescribed_displacements{
        PrescribedDisplacement{dof_map.global_index(1, DisplacementComponent::z), 0.0},
        PrescribedDisplacement{dof_map.global_index(2, DisplacementComponent::z), 0.0},
        PrescribedDisplacement{dof_map.global_index(3, DisplacementComponent::z), 0.0},
        PrescribedDisplacement{dof_map.global_index(4, DisplacementComponent::z), 0.0},
        PrescribedDisplacement{dof_map.global_index(1, DisplacementComponent::x), 0.0},
        PrescribedDisplacement{dof_map.global_index(4, DisplacementComponent::x), 0.0},
        PrescribedDisplacement{dof_map.global_index(5, DisplacementComponent::x), 0.0},
        PrescribedDisplacement{dof_map.global_index(8, DisplacementComponent::x), 0.0},
        PrescribedDisplacement{dof_map.global_index(1, DisplacementComponent::y), 0.0},
        PrescribedDisplacement{dof_map.global_index(2, DisplacementComponent::y), 0.0},
        PrescribedDisplacement{dof_map.global_index(5, DisplacementComponent::y), 0.0},
        PrescribedDisplacement{dof_map.global_index(6, DisplacementComponent::y), 0.0},
    };

    const auto result =
        solve_h8_model(elements, nodes, materials, dof_map, point_loads, prescribed_displacements);

    constexpr double tolerance = 1.0e-12;
    for (const auto &node : nodes.nodes())
    {
        const double expected_x = 0.0025 * node.x();
        const double expected_y = 0.0025 * node.y();
        const double expected_z = -0.01 * node.z();
        EXPECT_NEAR(result.displacements[dof_map.global_index(node.id(), DisplacementComponent::x)],
                    expected_x, tolerance);
        EXPECT_NEAR(result.displacements[dof_map.global_index(node.id(), DisplacementComponent::y)],
                    expected_y, tolerance);
        EXPECT_NEAR(result.displacements[dof_map.global_index(node.id(), DisplacementComponent::z)],
                    expected_z, tolerance);
    }

    double bottom_reaction = 0.0;
    for (const std::size_t node_id : {1U, 2U, 3U, 4U})
    {
        bottom_reaction +=
            result.reactions[dof_map.global_index(node_id, DisplacementComponent::z)];
    }
    EXPECT_NEAR(bottom_reaction, 10.0, tolerance);
}
} // namespace
