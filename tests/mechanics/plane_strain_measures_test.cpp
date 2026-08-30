#include "finelemethod/mechanics/plane_strain_measures.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::mechanics::calculate_plane_strain_measures;

TEST(PlaneStrainMeasures, CalculatesThreeDimensionalInvariants)
{
    const auto measures = calculate_plane_strain_measures(12.0, 4.0, 4.0, 0.0);

    EXPECT_DOUBLE_EQ(measures.von_mises, 8.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[0], 12.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[1], 4.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[2], 4.0);
}

TEST(PlaneStrainMeasures, IncludesShearAndSortsPrincipalStresses)
{
    const auto measures = calculate_plane_strain_measures(2.0, 2.0, 5.0, 3.0);

    EXPECT_NEAR(measures.von_mises, 6.0, 1.0e-12);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[0], 5.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[1], 5.0);
    EXPECT_DOUBLE_EQ(measures.principal_stresses[2], -1.0);
}

TEST(PlaneStrainMeasures, RejectsNonfiniteStressComponents)
{
    const double infinity = std::numeric_limits<double>::infinity();

    EXPECT_THROW(static_cast<void>(calculate_plane_strain_measures(infinity, 0.0, 0.0, 0.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(calculate_plane_strain_measures(0.0, infinity, 0.0, 0.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(calculate_plane_strain_measures(0.0, 0.0, infinity, 0.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(calculate_plane_strain_measures(0.0, 0.0, 0.0, infinity)),
                 std::invalid_argument);
}
} // namespace
