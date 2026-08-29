#include "finelemethod/model/q4_edge_pressure_load.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::model::Q4Edge;
using finelemethod::model::Q4EdgePressureLoad;

TEST(Q4EdgePressureLoad, StoresElementEdgeAndPressure)
{
    const Q4EdgePressureLoad load(12, Q4Edge::three, -25.5);

    EXPECT_EQ(load.element_id(), 12U);
    EXPECT_EQ(load.edge(), Q4Edge::three);
    EXPECT_DOUBLE_EQ(load.pressure(), -25.5);
}

TEST(Q4EdgePressureLoad, RejectsInvalidEdgeAndNonfinitePressure)
{
    const double infinity = std::numeric_limits<double>::infinity();

    EXPECT_THROW(Q4EdgePressureLoad(1, static_cast<Q4Edge>(4), 1.0), std::invalid_argument);
    EXPECT_THROW(Q4EdgePressureLoad(1, Q4Edge::one, infinity), std::invalid_argument);
}
} // namespace
