#include "finelemethod/model/point_load.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::model::DisplacementComponent;
using finelemethod::model::PointLoad;

TEST(PointLoad, StoresNodeComponentAndMagnitude)
{
    const PointLoad point_load(12, DisplacementComponent::y, -25.5);

    EXPECT_EQ(point_load.node_id(), 12);
    EXPECT_EQ(point_load.component(), DisplacementComponent::y);
    EXPECT_DOUBLE_EQ(point_load.magnitude(), -25.5);
}

TEST(PointLoad, RejectsNonfiniteMagnitude)
{
    const double infinity = std::numeric_limits<double>::infinity();
    const double not_a_number = std::numeric_limits<double>::quiet_NaN();

    EXPECT_THROW(PointLoad(1, DisplacementComponent::x, infinity), std::invalid_argument);
    EXPECT_THROW(PointLoad(1, DisplacementComponent::x, not_a_number), std::invalid_argument);
}
} // namespace
