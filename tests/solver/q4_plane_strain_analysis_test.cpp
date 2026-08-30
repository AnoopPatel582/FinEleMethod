#include "finelemethod/solver/q4_plane_strain_analysis.hpp"

#include <gtest/gtest.h>

#include <array>

namespace
{
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::IsotropicElasticMaterial;
using finelemethod::model::MaterialCollection;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::PointLoad;
using finelemethod::model::Q4Edge;
using finelemethod::model::Q4EdgePressureLoad;
using finelemethod::model::Q4Element;
using finelemethod::model::Q4ElementCollection;
using finelemethod::model::Q4NodeIds;
using finelemethod::model::SpatialDimension;
using finelemethod::solver::PrescribedDisplacement;
using finelemethod::solver::solve_q4_plane_strain_model;

TEST(Q4PlaneStrainAnalysis, ReproducesUniformUniaxialInPlaneStress)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0));
    nodes.add(Node(2, 1.0, 0.0));
    nodes.add(Node(3, 1.0, 1.0));
    nodes.add(Node(4, 0.0, 1.0));
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);

    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(1, 1000.0, 0.25));

    Q4ElementCollection elements;
    elements.add(Q4Element(1, Q4NodeIds{{1, 2, 3, 4}}, 1, 1.0));

    const std::array point_loads{
        PointLoad(2, DisplacementComponent::x, 2.5),
        PointLoad(3, DisplacementComponent::x, 2.5),
    };
    const std::array pressure_loads{Q4EdgePressureLoad(1, Q4Edge::two, -5.0)};
    const std::array prescribed_displacements{
        PrescribedDisplacement{dof_map.global_index(1, DisplacementComponent::x), 0.0},
        PrescribedDisplacement{dof_map.global_index(1, DisplacementComponent::y), 0.0},
        PrescribedDisplacement{dof_map.global_index(4, DisplacementComponent::x), 0.0},
    };

    const auto result = solve_q4_plane_strain_model(
        elements, nodes, materials, dof_map, point_loads, pressure_loads, prescribed_displacements);

    constexpr double axial_displacement = 0.009375;
    constexpr double transverse_displacement = -0.003125;
    constexpr double tolerance = 1.0e-12;
    EXPECT_NEAR(result.displacements[dof_map.global_index(1, DisplacementComponent::x)], 0.0,
                tolerance);
    EXPECT_NEAR(result.displacements[dof_map.global_index(1, DisplacementComponent::y)], 0.0,
                tolerance);
    EXPECT_NEAR(result.displacements[dof_map.global_index(2, DisplacementComponent::x)],
                axial_displacement, tolerance);
    EXPECT_NEAR(result.displacements[dof_map.global_index(2, DisplacementComponent::y)], 0.0,
                tolerance);
    EXPECT_NEAR(result.displacements[dof_map.global_index(3, DisplacementComponent::x)],
                axial_displacement, tolerance);
    EXPECT_NEAR(result.displacements[dof_map.global_index(3, DisplacementComponent::y)],
                transverse_displacement, tolerance);
    EXPECT_NEAR(result.displacements[dof_map.global_index(4, DisplacementComponent::x)], 0.0,
                tolerance);
    EXPECT_NEAR(result.displacements[dof_map.global_index(4, DisplacementComponent::y)],
                transverse_displacement, tolerance);

    EXPECT_NEAR(result.reactions[dof_map.global_index(1, DisplacementComponent::x)], -5.0,
                tolerance);
    EXPECT_NEAR(result.reactions[dof_map.global_index(4, DisplacementComponent::x)], -5.0,
                tolerance);

    ASSERT_EQ(result.element_results.size(), 1U);
    EXPECT_EQ(result.element_results[0].element_id, 1U);
    for (const auto &point : result.element_results[0].gauss_points)
    {
        EXPECT_NEAR(point.strain[0], axial_displacement, tolerance);
        EXPECT_NEAR(point.strain[1], transverse_displacement, tolerance);
        EXPECT_NEAR(point.strain[2], 0.0, tolerance);
        EXPECT_NEAR(point.strain[3], 0.0, tolerance);
        EXPECT_NEAR(point.stress[0], 10.0, tolerance);
        EXPECT_NEAR(point.stress[1], 0.0, tolerance);
        EXPECT_NEAR(point.stress[2], 2.5, tolerance);
        EXPECT_NEAR(point.stress[3], 0.0, tolerance);
    }
}
} // namespace
