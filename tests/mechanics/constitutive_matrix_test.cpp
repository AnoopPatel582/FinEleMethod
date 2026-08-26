#include "finelemethod/mechanics/constitutive_matrix.hpp"

#include "finelemethod/math/dense_vector.hpp"

#include <gtest/gtest.h>

namespace
{
using finelemethod::math::DenseVector;
using finelemethod::mechanics::plane_stress_constitutive_matrix;
using finelemethod::model::IsotropicElasticMaterial;

TEST(PlaneStressConstitutiveMatrix, ReturnsExpectedThreeByThreeMatrix)
{
    const IsotropicElasticMaterial material(1, 100.0, 0.25);

    const auto matrix = plane_stress_constitutive_matrix(material);

    ASSERT_EQ(matrix.rows(), 3);
    ASSERT_EQ(matrix.columns(), 3);

    const double factor = 100.0 / (1.0 - 0.25 * 0.25);
    EXPECT_DOUBLE_EQ(matrix(0, 0), factor);
    EXPECT_DOUBLE_EQ(matrix(0, 1), factor * 0.25);
    EXPECT_DOUBLE_EQ(matrix(1, 0), factor * 0.25);
    EXPECT_DOUBLE_EQ(matrix(1, 1), factor);
    EXPECT_DOUBLE_EQ(matrix(2, 2), factor * (1.0 - 0.25) / 2.0);
}

TEST(PlaneStressConstitutiveMatrix, HasZeroNormalShearCoupling)
{
    const IsotropicElasticMaterial material(1, 210.0e9, 0.3);

    const auto matrix = plane_stress_constitutive_matrix(material);

    EXPECT_DOUBLE_EQ(matrix(0, 2), 0.0);
    EXPECT_DOUBLE_EQ(matrix(1, 2), 0.0);
    EXPECT_DOUBLE_EQ(matrix(2, 0), 0.0);
    EXPECT_DOUBLE_EQ(matrix(2, 1), 0.0);
}

TEST(PlaneStressConstitutiveMatrix, ConvertsUniaxialStressStateFromCompatibleStrain)
{
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    const auto matrix = plane_stress_constitutive_matrix(material);
    DenseVector strain(3);
    strain[0] = 0.01;
    strain[1] = -0.25 * 0.01;

    const DenseVector stress = matrix * strain;

    EXPECT_NEAR(stress[0], 2.0, 1.0e-12);
    EXPECT_NEAR(stress[1], 0.0, 1.0e-12);
    EXPECT_NEAR(stress[2], 0.0, 1.0e-12);
}

TEST(PlaneStressConstitutiveMatrix, ProducesCorrectPureShearStress)
{
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    const auto matrix = plane_stress_constitutive_matrix(material);
    DenseVector strain(3);
    strain[2] = 0.01;

    const DenseVector stress = matrix * strain;

    const double shear_modulus = 200.0 / (2.0 * (1.0 + 0.25));
    EXPECT_NEAR(stress[0], 0.0, 1.0e-12);
    EXPECT_NEAR(stress[1], 0.0, 1.0e-12);
    EXPECT_NEAR(stress[2], shear_modulus * 0.01, 1.0e-12);
}
} // namespace
