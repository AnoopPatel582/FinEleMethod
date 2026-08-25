#include "finelemethod/assembly/load_assembly.hpp"

#include <gtest/gtest.h>

#include <array>
#include <stdexcept>

namespace
{
using finelemethod::assembly::assemble_point_load_vector;
using finelemethod::model::DisplacementComponent;
using finelemethod::model::DofMap;
using finelemethod::model::Node;
using finelemethod::model::NodeCollection;
using finelemethod::model::PointLoad;
using finelemethod::model::SpatialDimension;

NodeCollection make_nodes()
{
    NodeCollection nodes;
    nodes.add(Node(10, 0.0, 0.0));
    nodes.add(Node(20, 1.0, 0.0));
    return nodes;
}

TEST(PointLoadAssembly, MapsAndAccumulatesTwoDimensionalLoads)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const std::array point_loads{PointLoad(10, DisplacementComponent::x, 5.0),
                                 PointLoad(20, DisplacementComponent::y, -3.0),
                                 PointLoad(10, DisplacementComponent::x, 2.5)};

    const auto load_vector = assemble_point_load_vector(dof_map, point_loads);

    EXPECT_EQ(load_vector.size(), 4);
    EXPECT_DOUBLE_EQ(load_vector[0], 7.5);
    EXPECT_DOUBLE_EQ(load_vector[1], 0.0);
    EXPECT_DOUBLE_EQ(load_vector[2], 0.0);
    EXPECT_DOUBLE_EQ(load_vector[3], -3.0);
}

TEST(PointLoadAssembly, MapsThreeDimensionalZLoad)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::three_dimensional);
    const std::array point_loads{PointLoad(20, DisplacementComponent::z, 8.0)};

    const auto load_vector = assemble_point_load_vector(dof_map, point_loads);

    EXPECT_EQ(load_vector.size(), 6);
    EXPECT_DOUBLE_EQ(load_vector[5], 8.0);
}

TEST(PointLoadAssembly, EmptyLoadCollectionProducesZeroVector)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const std::array<PointLoad, 0> point_loads{};

    const auto load_vector = assemble_point_load_vector(dof_map, point_loads);

    for (std::size_t index = 0; index < load_vector.size(); ++index)
    {
        EXPECT_DOUBLE_EQ(load_vector[index], 0.0);
    }
}

TEST(PointLoadAssembly, RejectsUnknownNodeId)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const std::array point_loads{PointLoad(99, DisplacementComponent::x, 1.0)};

    EXPECT_THROW(static_cast<void>(assemble_point_load_vector(dof_map, point_loads)),
                 std::out_of_range);
}

TEST(PointLoadAssembly, RejectsZLoadInTwoDimensionalModel)
{
    const NodeCollection nodes = make_nodes();
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const std::array point_loads{PointLoad(10, DisplacementComponent::z, 1.0)};

    EXPECT_THROW(static_cast<void>(assemble_point_load_vector(dof_map, point_loads)),
                 std::invalid_argument);
}

TEST(PointLoadAssembly, RejectsModelWithoutDegreesOfFreedom)
{
    const NodeCollection nodes;
    const DofMap dof_map(nodes, SpatialDimension::two_dimensional);
    const std::array<PointLoad, 0> point_loads{};

    EXPECT_THROW(static_cast<void>(assemble_point_load_vector(dof_map, point_loads)),
                 std::invalid_argument);
}
} // namespace
