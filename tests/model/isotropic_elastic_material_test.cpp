#include "finelemethod/model/isotropic_elastic_material.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::model::IsotropicElasticMaterial;

TEST(IsotropicElasticMaterial, StoresMaterialProperties)
{
    const IsotropicElasticMaterial steel(7, 210.0e9, 0.3);

    EXPECT_EQ(steel.id(), 7);
    EXPECT_DOUBLE_EQ(steel.youngs_modulus(), 210.0e9);
    EXPECT_DOUBLE_EQ(steel.poisson_ratio(), 0.3);
}

TEST(IsotropicElasticMaterial, AcceptsValidPoissonRatioNearBothLimits)
{
    EXPECT_NO_THROW(IsotropicElasticMaterial(1, 1.0, -0.999));
    EXPECT_NO_THROW(IsotropicElasticMaterial(2, 1.0, 0.499));
}

TEST(IsotropicElasticMaterial, RejectsNonpositiveOrNonfiniteYoungsModulus)
{
    const double infinity = std::numeric_limits<double>::infinity();
    const double not_a_number = std::numeric_limits<double>::quiet_NaN();

    EXPECT_THROW(IsotropicElasticMaterial(1, 0.0, 0.3), std::invalid_argument);
    EXPECT_THROW(IsotropicElasticMaterial(1, -1.0, 0.3), std::invalid_argument);
    EXPECT_THROW(IsotropicElasticMaterial(1, infinity, 0.3), std::invalid_argument);
    EXPECT_THROW(IsotropicElasticMaterial(1, not_a_number, 0.3), std::invalid_argument);
}

TEST(IsotropicElasticMaterial, RejectsPoissonRatioOutsideOpenPhysicalRange)
{
    const double infinity = std::numeric_limits<double>::infinity();
    const double not_a_number = std::numeric_limits<double>::quiet_NaN();

    EXPECT_THROW(IsotropicElasticMaterial(1, 1.0, -1.0), std::invalid_argument);
    EXPECT_THROW(IsotropicElasticMaterial(1, 1.0, 0.5), std::invalid_argument);
    EXPECT_THROW(IsotropicElasticMaterial(1, 1.0, -1.1), std::invalid_argument);
    EXPECT_THROW(IsotropicElasticMaterial(1, 1.0, 0.6), std::invalid_argument);
    EXPECT_THROW(IsotropicElasticMaterial(1, 1.0, infinity), std::invalid_argument);
    EXPECT_THROW(IsotropicElasticMaterial(1, 1.0, not_a_number), std::invalid_argument);
}
} // namespace
