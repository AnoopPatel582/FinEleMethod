#include "finelemethod/mechanics/solid_stress_measures.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::mechanics::calculate_solid_stress_measures;

TEST(SolidStressMeasures, CalculatesUniaxialStress)
{
    const auto measures = calculate_solid_stress_measures(10.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    EXPECT_NEAR(measures.von_mises, 10.0, 1.0e-14);
    EXPECT_NEAR(measures.principal_stresses[0], 10.0, 1.0e-14);
    EXPECT_NEAR(measures.principal_stresses[1], 0.0, 1.0e-14);
    EXPECT_NEAR(measures.principal_stresses[2], 0.0, 1.0e-14);
}

TEST(SolidStressMeasures, CalculatesHydrostaticStress)
{
    const auto measures = calculate_solid_stress_measures(-5.0, -5.0, -5.0, 0.0, 0.0, 0.0);

    EXPECT_DOUBLE_EQ(measures.von_mises, 0.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[0], -5.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[1], -5.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[2], -5.0);
}

TEST(SolidStressMeasures, CalculatesThreeDimensionalPureShear)
{
    const auto measures = calculate_solid_stress_measures(0.0, 0.0, 0.0, 3.0, 4.0, 0.0);

    EXPECT_NEAR(measures.von_mises, 5.0 * std::sqrt(3.0), 1.0e-14);
    EXPECT_NEAR(measures.principal_stresses[0], 5.0, 1.0e-14);
    EXPECT_NEAR(measures.principal_stresses[1], 0.0, 1.0e-14);
    EXPECT_NEAR(measures.principal_stresses[2], -5.0, 1.0e-14);
}

TEST(SolidStressMeasures, RejectsNonfiniteStressComponents)
{
    EXPECT_THROW(static_cast<void>(calculate_solid_stress_measures(
                     1.0, 2.0, 3.0, std::numeric_limits<double>::infinity(), 0.0, 0.0)),
                 std::invalid_argument);
}
} // namespace
