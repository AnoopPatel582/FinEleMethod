#include "finelemethod/elements/q4_plane_stress_stiffness.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

namespace
{
using finelemethod::elements::q4_plane_stress_stiffness_matrix;
using finelemethod::elements::Q4NodeCoordinates;
using finelemethod::model::IsotropicElasticMaterial;
using finelemethod::model::MaterialCollection;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::Q4Element;
using finelemethod::model::Q4NodeIds;

TEST(Q4PlaneStressModelStiffness, ResolvesElementNodesMaterialAndThickness)
{
    NodeCollection nodes;
    nodes.add(Node(40, 0.0, 1.0));
    nodes.add(Node(10, 0.0, 0.0));
    nodes.add(Node(30, 2.0, 1.0));
    nodes.add(Node(20, 2.0, 0.0));

    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(5, 70.0e9, 0.25));
    materials.add(IsotropicElasticMaterial(9, 210.0e9, 0.3));

    const Q4Element element(1, Q4NodeIds{{10, 20, 30, 40}}, 9, 0.02);
    const auto resolved = q4_plane_stress_stiffness_matrix(element, nodes, materials);
    const Q4NodeCoordinates coordinates{{{{0.0, 0.0}}, {{2.0, 0.0}}, {{2.0, 1.0}}, {{0.0, 1.0}}}};
    const auto expected =
        q4_plane_stress_stiffness_matrix(coordinates, materials.at(9), element.thickness());

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

TEST(Q4PlaneStressModelStiffness, RejectsElementReferencingMissingNode)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0));
    nodes.add(Node(2, 1.0, 0.0));
    nodes.add(Node(3, 1.0, 1.0));

    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(1, 1000.0, 0.2));
    const Q4Element element(1, Q4NodeIds{{1, 2, 3, 4}}, 1, 1.0);

    EXPECT_THROW(static_cast<void>(q4_plane_stress_stiffness_matrix(element, nodes, materials)),
                 std::out_of_range);
}

TEST(Q4PlaneStressModelStiffness, RejectsElementReferencingMissingMaterial)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0));
    nodes.add(Node(2, 1.0, 0.0));
    nodes.add(Node(3, 1.0, 1.0));
    nodes.add(Node(4, 0.0, 1.0));

    const MaterialCollection materials;
    const Q4Element element(1, Q4NodeIds{{1, 2, 3, 4}}, 7, 1.0);

    EXPECT_THROW(static_cast<void>(q4_plane_stress_stiffness_matrix(element, nodes, materials)),
                 std::out_of_range);
}
} // namespace
