#include "finelemethod/assembly/q4_stiffness_assembly.hpp"

#include "finelemethod/assembly/q4_dof_mapping.hpp"
#include "finelemethod/elements/q4_plane_strain_stiffness.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::assembly::assemble_q4_plane_strain_stiffness;
using finelemethod::assembly::q4_global_dof_indices;
using finelemethod::elements::q4_plane_strain_stiffness_matrix;
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::IsotropicElasticMaterial;
using finelemethod::model::MaterialCollection;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::Q4Element;
using finelemethod::model::Q4ElementCollection;
using finelemethod::model::Q4NodeIds;
using finelemethod::model::SpatialDimension;

MaterialCollection make_materials()
{
    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(1, 210.0e9, 0.3));
    return materials;
}

TEST(Q4PlaneStrainStiffnessAssembly, PlacesSingleElementMatrixInGlobalDofOrder)
{
    NodeCollection nodes;
    nodes.add(Node(30, 2.0, 1.0));
    nodes.add(Node(10, 0.0, 0.0));
    nodes.add(Node(40, 0.0, 1.0));
    nodes.add(Node(20, 2.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const auto materials = make_materials();
    const Q4Element element(1, Q4NodeIds{{10, 20, 30, 40}}, 1, 0.02);
    Q4ElementCollection elements;
    elements.add(element);

    const auto global = assemble_q4_plane_strain_stiffness(elements, nodes, materials, dof_map);
    const auto local = q4_plane_strain_stiffness_matrix(element, nodes, materials);
    const auto indices = q4_global_dof_indices(element, dof_map);

    ASSERT_EQ(global.rows(), dof_map.size());
    ASSERT_EQ(global.columns(), dof_map.size());
    for (std::size_t local_row = 0; local_row < indices.size(); ++local_row)
    {
        for (std::size_t local_column = 0; local_column < indices.size(); ++local_column)
        {
            EXPECT_DOUBLE_EQ(global(indices[local_row], indices[local_column]),
                             local(local_row, local_column));
        }
    }
}

TEST(Q4PlaneStrainStiffnessAssembly, AccumulatesContributionsAtSharedNodes)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0));
    nodes.add(Node(2, 1.0, 0.0));
    nodes.add(Node(3, 2.0, 0.0));
    nodes.add(Node(4, 0.0, 1.0));
    nodes.add(Node(5, 1.0, 1.0));
    nodes.add(Node(6, 2.0, 1.0));
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const auto materials = make_materials();
    const Q4Element left(1, Q4NodeIds{{1, 2, 5, 4}}, 1, 0.01);
    const Q4Element right(2, Q4NodeIds{{2, 3, 6, 5}}, 1, 0.01);
    Q4ElementCollection elements;
    elements.add(left);
    elements.add(right);

    const auto global = assemble_q4_plane_strain_stiffness(elements, nodes, materials, dof_map);
    const auto left_stiffness = q4_plane_strain_stiffness_matrix(left, nodes, materials);
    const auto right_stiffness = q4_plane_strain_stiffness_matrix(right, nodes, materials);
    const auto node_2_x = dof_map.global_index(2, DisplacementComponent::x);
    const auto node_5_x = dof_map.global_index(5, DisplacementComponent::x);

    EXPECT_DOUBLE_EQ(global(node_2_x, node_2_x), left_stiffness(2, 2) + right_stiffness(0, 0));
    EXPECT_DOUBLE_EQ(global(node_2_x, node_5_x), left_stiffness(2, 4) + right_stiffness(0, 6));
}

TEST(Q4PlaneStrainStiffnessAssembly, RejectsThreeDimensionalDofMap)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);

    EXPECT_THROW(static_cast<void>(assemble_q4_plane_strain_stiffness(
                     Q4ElementCollection{}, nodes, MaterialCollection{}, dof_map)),
                 std::invalid_argument);
}
} // namespace
