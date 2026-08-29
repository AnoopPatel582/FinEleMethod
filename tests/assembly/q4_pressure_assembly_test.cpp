#include "finelemethod/assembly/q4_pressure_assembly.hpp"

#include <gtest/gtest.h>

#include <array>
#include <stdexcept>

namespace
{
using finelemethod::assembly::assemble_q4_edge_pressure_loads;
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::Q4Edge;
using finelemethod::model::Q4EdgePressureLoad;
using finelemethod::model::Q4Element;
using finelemethod::model::Q4ElementCollection;
using finelemethod::model::Q4NodeIds;
using finelemethod::model::SpatialDimension;

TEST(Q4PressureAssembly, MapsAndAccumulatesElementLoadsInGlobalDofOrder)
{
    NodeCollection nodes;
    nodes.add(Node(30, 4.0, 2.0));
    nodes.add(Node(10, 0.0, 0.0));
    nodes.add(Node(40, 0.0, 2.0));
    nodes.add(Node(20, 4.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    Q4ElementCollection elements;
    elements.add(Q4Element(7, Q4NodeIds{{10, 20, 30, 40}}, 1, 0.5));
    const std::array loads{Q4EdgePressureLoad(7, Q4Edge::two, 10.0),
                           Q4EdgePressureLoad(7, Q4Edge::two, 2.0),
                           Q4EdgePressureLoad(7, Q4Edge::one, 4.0)};

    const auto global = assemble_q4_edge_pressure_loads(elements, nodes, dof_map, loads);

    EXPECT_DOUBLE_EQ(global[dof_map.global_index(20, DisplacementComponent::x)], -6.0);
    EXPECT_DOUBLE_EQ(global[dof_map.global_index(30, DisplacementComponent::x)], -6.0);
    EXPECT_DOUBLE_EQ(global[dof_map.global_index(10, DisplacementComponent::y)], 4.0);
    EXPECT_DOUBLE_EQ(global[dof_map.global_index(20, DisplacementComponent::y)], 4.0);
}

TEST(Q4PressureAssembly, EmptyLoadCollectionProducesZeroVector)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const std::array<Q4EdgePressureLoad, 0> loads{};

    const auto global =
        assemble_q4_edge_pressure_loads(Q4ElementCollection{}, nodes, dof_map, loads);

    ASSERT_EQ(global.size(), 2U);
    EXPECT_DOUBLE_EQ(global[0], 0.0);
    EXPECT_DOUBLE_EQ(global[1], 0.0);
}

TEST(Q4PressureAssembly, RejectsUnknownElement)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0));
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const std::array loads{Q4EdgePressureLoad(99, Q4Edge::one, 1.0)};

    EXPECT_THROW(static_cast<void>(
                     assemble_q4_edge_pressure_loads(Q4ElementCollection{}, nodes, dof_map, loads)),
                 std::out_of_range);
}

TEST(Q4PressureAssembly, RejectsThreeDimensionalDofMapAndEmptyModel)
{
    NodeCollection nodes;
    nodes.add(Node(1, 0.0, 0.0, 0.0));
    const DofMap three_dimensional(nodes, SpatialDimension::three_dimensional);
    const NodeCollection empty_nodes;
    const DofMap empty(empty_nodes, SpatialDimension::two_dimensional);
    const std::array<Q4EdgePressureLoad, 0> loads{};

    EXPECT_THROW(static_cast<void>(assemble_q4_edge_pressure_loads(Q4ElementCollection{}, nodes,
                                                                   three_dimensional, loads)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(assemble_q4_edge_pressure_loads(Q4ElementCollection{},
                                                                   empty_nodes, empty, loads)),
                 std::invalid_argument);
}
} // namespace
