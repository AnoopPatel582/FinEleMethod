#include "finelemethod/elements/h8_stiffness.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::elements::h8_stiffness_matrix;
using finelemethod::elements::H8NodeCoordinates;
using finelemethod::model::H8Element;
using finelemethod::model::H8NodeIds;
using finelemethod::model::IsotropicElasticMaterial;
using finelemethod::model::MaterialCollection;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;

NodeCollection box_nodes()
{
    NodeCollection nodes;
    nodes.add(Node(70, 2.0, 4.0, 6.0));
    nodes.add(Node(10, 0.0, 0.0, 0.0));
    nodes.add(Node(50, 0.0, 0.0, 6.0));
    nodes.add(Node(30, 2.0, 4.0, 0.0));
    nodes.add(Node(80, 0.0, 4.0, 6.0));
    nodes.add(Node(20, 2.0, 0.0, 0.0));
    nodes.add(Node(60, 2.0, 0.0, 6.0));
    nodes.add(Node(40, 0.0, 4.0, 0.0));
    return nodes;
}

TEST(H8ModelStiffness, ResolvesElementNodesAndMaterial)
{
    const NodeCollection nodes = box_nodes();
    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(5, 70.0e9, 0.25));
    materials.add(IsotropicElasticMaterial(9, 210.0e9, 0.3));
    const H8Element element(1, H8NodeIds{{10, 20, 30, 40, 50, 60, 70, 80}}, 9);

    const auto resolved = h8_stiffness_matrix(element, nodes, materials);
    const H8NodeCoordinates coordinates{{{{0.0, 0.0, 0.0}},
                                         {{2.0, 0.0, 0.0}},
                                         {{2.0, 4.0, 0.0}},
                                         {{0.0, 4.0, 0.0}},
                                         {{0.0, 0.0, 6.0}},
                                         {{2.0, 0.0, 6.0}},
                                         {{2.0, 4.0, 6.0}},
                                         {{0.0, 4.0, 6.0}}}};
    const auto expected = h8_stiffness_matrix(coordinates, materials.at(9));

    ASSERT_EQ(resolved.rows(), expected.rows());
    ASSERT_EQ(resolved.columns(), expected.columns());
    for (std::size_t row = 0; row < resolved.rows(); ++row)
    {
        for (std::size_t column = 0; column < resolved.columns(); ++column)
        {
            EXPECT_DOUBLE_EQ(resolved(row, column), expected(row, column));
        }
    }
}

TEST(H8ModelStiffness, RejectsElementReferencingMissingNode)
{
    NodeCollection nodes = box_nodes();
    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(1, 1000.0, 0.2));
    const H8Element element(1, H8NodeIds{{10, 20, 30, 40, 50, 60, 70, 99}}, 1);

    EXPECT_THROW(static_cast<void>(h8_stiffness_matrix(element, nodes, materials)),
                 std::out_of_range);
}

TEST(H8ModelStiffness, RejectsElementReferencingMissingMaterial)
{
    const NodeCollection nodes = box_nodes();
    const MaterialCollection materials;
    const H8Element element(1, H8NodeIds{{10, 20, 30, 40, 50, 60, 70, 80}}, 7);

    EXPECT_THROW(static_cast<void>(h8_stiffness_matrix(element, nodes, materials)),
                 std::out_of_range);
}
} // namespace
