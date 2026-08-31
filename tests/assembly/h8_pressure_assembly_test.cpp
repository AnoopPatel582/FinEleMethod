#include "finelemethod/assembly/h8_pressure_assembly.hpp"

#include <gtest/gtest.h>

#include <array>
#include <stdexcept>

namespace
{
using finelemethod::assembly::assemble_h8_face_pressure_loads;
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::H8Element;
using finelemethod::model::H8ElementCollection;
using finelemethod::model::H8Face;
using finelemethod::model::H8FacePressureLoad;
using finelemethod::model::H8NodeIds;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::SpatialDimension;

TEST(H8PressureAssembly, MapsAndAccumulatesElementLoadsInGlobalDofOrder)
{
    NodeCollection nodes;
    nodes.add(Node(70, 2.0, 3.0, 4.0));
    nodes.add(Node(10, 0.0, 0.0, 0.0));
    nodes.add(Node(80, 0.0, 3.0, 4.0));
    nodes.add(Node(20, 2.0, 0.0, 0.0));
    nodes.add(Node(50, 0.0, 0.0, 4.0));
    nodes.add(Node(30, 2.0, 3.0, 0.0));
    nodes.add(Node(60, 2.0, 0.0, 4.0));
    nodes.add(Node(40, 0.0, 3.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);
    H8ElementCollection elements;
    elements.add(H8Element(7, H8NodeIds{{10, 20, 30, 40, 50, 60, 70, 80}}, 1));
    const std::array loads{H8FacePressureLoad(7, H8Face::two, 10.0),
                           H8FacePressureLoad(7, H8Face::two, 2.0),
                           H8FacePressureLoad(7, H8Face::four, 4.0)};

    const auto global = assemble_h8_face_pressure_loads(elements, nodes, dof_map, loads);

    for (const std::size_t node_id : {50U, 60U, 70U, 80U})
    {
        EXPECT_NEAR(global[dof_map.global_index(node_id, DisplacementComponent::z)], -18.0,
                    1.0e-12);
    }
    for (const std::size_t node_id : {20U, 30U, 60U, 70U})
    {
        EXPECT_NEAR(global[dof_map.global_index(node_id, DisplacementComponent::x)], -12.0,
                    1.0e-12);
    }
}

TEST(H8PressureAssembly, EmptyLoadCollectionProducesZeroVector)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);
    const std::array<H8FacePressureLoad, 0> loads{};

    const auto global =
        assemble_h8_face_pressure_loads(H8ElementCollection{}, nodes, dof_map, loads);

    ASSERT_EQ(global.size(), 3U);
    EXPECT_DOUBLE_EQ(global[0], 0.0);
    EXPECT_DOUBLE_EQ(global[1], 0.0);
    EXPECT_DOUBLE_EQ(global[2], 0.0);
}

TEST(H8PressureAssembly, RejectsUnknownElement)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);
    const std::array loads{H8FacePressureLoad(99, H8Face::one, 1.0)};

    EXPECT_THROW(static_cast<void>(
                     assemble_h8_face_pressure_loads(H8ElementCollection{}, nodes, dof_map, loads)),
                 std::out_of_range);
}

TEST(H8PressureAssembly, RejectsTwoDimensionalDofMapAndEmptyModel)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0));
    const DofMap two_dimensional(nodes, SpatialDimension::two_dimensional);
    const NodeCollection empty_nodes;
    const DofMap empty(empty_nodes, SpatialDimension::three_dimensional);
    const std::array<H8FacePressureLoad, 0> loads{};

    EXPECT_THROW(static_cast<void>(assemble_h8_face_pressure_loads(H8ElementCollection{}, nodes,
                                                                   two_dimensional, loads)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(assemble_h8_face_pressure_loads(H8ElementCollection{},
                                                                   empty_nodes, empty, loads)),
                 std::invalid_argument);
}
} // namespace
