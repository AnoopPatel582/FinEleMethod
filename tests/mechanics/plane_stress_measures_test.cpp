#include "finelemethod/mechanics/plane_stress_measures.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::mechanics::calculate_plane_stress_measures;

TEST(PlaneStressMeasures, CalculatesUniaxialTension)
{
    const auto measures = calculate_plane_stress_measures(10.0, 0.0, 0.0);

    EXPECT_DOUBLE_EQ(measures.von_mises, 10.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[0], 10.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[1], 0.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[2], 0.0);
}

TEST(PlaneStressMeasures, IncludesAndSortsZeroOutOfPlanePrincipalStress)
{
    const auto measures = calculate_plane_stress_measures(-10.0, -4.0, 0.0);

    EXPECT_DOUBLE_EQ(measures.principal_stresses[0], 0.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[1], -4.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[2], -10.0);
}

TEST(PlaneStressMeasures, CalculatesPureShear)
{
    const auto measures = calculate_plane_stress_measures(0.0, 0.0, 5.0);

    EXPECT_DOUBLE_EQ(measures.von_mises, 5.0 * std::sqrt(3.0));
    EXPECT_DOUBLE_EQ(measures.principal_stresses[0], 5.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[1], 0.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[2], -5.0);
}

TEST(PlaneStressMeasures, RejectsNonfiniteStressComponents)
{
    EXPECT_THROW(static_cast<void>(calculate_plane_stress_measures(
                     std::numeric_limits<double>::infinity(), 0.0, 0.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(calculate_plane_stress_measures(
                     0.0, std::numeric_limits<double>::quiet_NaN(), 0.0)),
                 std::invalid_argument);
}
} // namespace
