#include "finelemethod/assembly/h8_stiffness_assembly.hpp"

#include "finelemethod/assembly/h8_dof_mapping.hpp"
#include "finelemethod/elements/h8_stiffness.hpp"
#include "finelemethod/math/csr_matrix.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::assembly::assemble_h8_stiffness;
using finelemethod::assembly::assemble_h8_stiffness_coo;
using finelemethod::assembly::h8_global_dof_indices;
using finelemethod::elements::h8_stiffness_matrix;
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::H8Element;
using finelemethod::model::H8ElementCollection;
using finelemethod::model::H8NodeIds;
using finelemethod::model::IsotropicElasticMaterial;
using finelemethod::model::MaterialCollection;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::SpatialDimension;

MaterialCollection make_materials()
{
    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(1, 210.0e9, 0.3));
    return materials;
}

NodeCollection make_two_element_nodes()
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0, 0.0));
    nodes.add(Node(2, 1.0, 0.0, 0.0));
    nodes.add(Node(3, 2.0, 0.0, 0.0));
    nodes.add(Node(4, 0.0, 1.0, 0.0));
    nodes.add(Node(5, 1.0, 1.0, 0.0));
    nodes.add(Node(6, 2.0, 1.0, 0.0));
    nodes.add(Node(7, 0.0, 0.0, 1.0));
    nodes.add(Node(8, 1.0, 0.0, 1.0));
    nodes.add(Node(9, 2.0, 0.0, 1.0));
    nodes.add(Node(10, 0.0, 1.0, 1.0));
    nodes.add(Node(11, 1.0, 1.0, 1.0));
    nodes.add(Node(12, 2.0, 1.0, 1.0));
    return nodes;
}

double csr_value(const finelemethod::math::CsrMatrix &matrix, const std::size_t row,
                 const std::size_t column)
{
    for (std::size_t index = matrix.row_offsets()[row]; index < matrix.row_offsets()[row + 1];
         ++index)
    {
        if (matrix.column_indices()[index] == column)
        {
            return matrix.values()[index];
        }
    }
    return 0.0;
}

TEST(H8StiffnessAssembly, PlacesSingleElementMatrixInGlobalDofOrder)
{
    NodeCollection nodes;
    nodes.add(Node(7, 1.0, 1.0, 1.0));
    nodes.add(Node(1, 0.0, 0.0, 0.0));
    nodes.add(Node(5, 0.0, 0.0, 1.0));
    nodes.add(Node(3, 1.0, 1.0, 0.0));
    nodes.add(Node(8, 0.0, 1.0, 1.0));
    nodes.add(Node(2, 1.0, 0.0, 0.0));
    nodes.add(Node(6, 1.0, 0.0, 1.0));
    nodes.add(Node(4, 0.0, 1.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);
    const auto materials = make_materials();
    const H8Element element(1, H8NodeIds{{1, 2, 3, 4, 5, 6, 7, 8}}, 1);
    H8ElementCollection elements;
    elements.add(element);

    const auto global = assemble_h8_stiffness(elements, nodes, materials, dof_map);
    const auto local = h8_stiffness_matrix(element, nodes, materials);
    const auto indices = h8_global_dof_indices(element, dof_map);

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

TEST(H8StiffnessAssembly, AccumulatesContributionsAtSharedNodes)
{
    const NodeCollection nodes = make_two_element_nodes();
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);
    const auto materials = make_materials();
    const H8Element left(1, H8NodeIds{{1, 2, 5, 4, 7, 8, 11, 10}}, 1);
    const H8Element right(2, H8NodeIds{{2, 3, 6, 5, 8, 9, 12, 11}}, 1);
    H8ElementCollection elements;
    elements.add(left);
    elements.add(right);

    const auto global = assemble_h8_stiffness(elements, nodes, materials, dof_map);
    const auto left_stiffness = h8_stiffness_matrix(left, nodes, materials);
    const auto right_stiffness = h8_stiffness_matrix(right, nodes, materials);
    const auto node_2_x = dof_map.global_index(2, DisplacementComponent::x);
    const auto node_8_x = dof_map.global_index(8, DisplacementComponent::x);

    EXPECT_DOUBLE_EQ(global(node_2_x, node_2_x), left_stiffness(3, 3) + right_stiffness(0, 0));
    EXPECT_DOUBLE_EQ(global(node_2_x, node_8_x), left_stiffness(3, 15) + right_stiffness(0, 12));
}

TEST(H8StiffnessAssembly, CooPathMatchesDenseAssemblyAfterCsrConversion)
{
    const NodeCollection nodes = make_two_element_nodes();
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);
    const auto materials = make_materials();
    H8ElementCollection elements;
    elements.add(H8Element(1, H8NodeIds{{1, 2, 5, 4, 7, 8, 11, 10}}, 1));
    elements.add(H8Element(2, H8NodeIds{{2, 3, 6, 5, 8, 9, 12, 11}}, 1));

    const auto dense = assemble_h8_stiffness(elements, nodes, materials, dof_map);
    const auto sparse = finelemethod::math::convert_to_csr(
        assemble_h8_stiffness_coo(elements, nodes, materials, dof_map));

    ASSERT_EQ(sparse.rows(), dense.rows());
    ASSERT_EQ(sparse.columns(), dense.columns());
    for (std::size_t row = 0; row < dense.rows(); ++row)
    {
        for (std::size_t column = 0; column < dense.columns(); ++column)
        {
            EXPECT_DOUBLE_EQ(csr_value(sparse, row, column), dense(row, column));
        }
    }
}

TEST(H8StiffnessAssembly, CooPathSupportsEmptyElementCollection)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);

    const auto sparse =
        assemble_h8_stiffness_coo(H8ElementCollection{}, nodes, MaterialCollection{}, dof_map);

    EXPECT_EQ(sparse.rows(), 3U);
    EXPECT_EQ(sparse.columns(), 3U);
    EXPECT_EQ(sparse.nonzero_count(), 0U);
}

TEST(H8StiffnessAssembly, EmptyElementCollectionProducesZeroMatrix)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);

    const auto global =
        assemble_h8_stiffness(H8ElementCollection{}, nodes, MaterialCollection{}, dof_map);

    ASSERT_EQ(global.rows(), 3);
    ASSERT_EQ(global.columns(), 3);
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            EXPECT_DOUBLE_EQ(global(row, column), 0.0);
        }
    }
}

TEST(H8StiffnessAssembly, RejectsTwoDimensionalDofMap)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);

    EXPECT_THROW(static_cast<void>(assemble_h8_stiffness(H8ElementCollection{}, nodes,
                                                         MaterialCollection{}, dof_map)),
                 std::invalid_argument);
}

TEST(H8StiffnessAssembly, RejectsModelWithoutDegreesOfFreedom)
{
    const NodeCollection nodes;
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);

    EXPECT_THROW(static_cast<void>(assemble_h8_stiffness(H8ElementCollection{}, nodes,
                                                         MaterialCollection{}, dof_map)),
                 std::invalid_argument);
}
} // namespace
