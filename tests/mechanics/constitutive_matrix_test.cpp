#include "finelemethod/mechanics/constitutive_matrix.hpp"

#include "finelemethod/math/dense_vector.hpp"

#include <gtest/gtest.h>

#include <cstddef>

namespace
{
using finelemethod::math::DenseVector;
using finelemethod::mechanics::plane_strain_constitutive_matrix;
using finelemethod::mechanics::plane_stress_constitutive_matrix;
using finelemethod::mechanics::solid_isotropic_constitutive_matrix;
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

TEST(PlaneStrainConstitutiveMatrix, ReturnsExpectedThreeByThreeMatrix)
{
    const IsotropicElasticMaterial material(1, 100.0, 0.25);

    const auto matrix = plane_strain_constitutive_matrix(material);

    ASSERT_EQ(matrix.rows(), 3U);
    ASSERT_EQ(matrix.columns(), 3U);
    const double factor = 100.0 / ((1.0 + 0.25) * (1.0 - 2.0 * 0.25));
    EXPECT_DOUBLE_EQ(matrix(0, 0), factor * (1.0 - 0.25));
    EXPECT_DOUBLE_EQ(matrix(0, 1), factor * 0.25);
    EXPECT_DOUBLE_EQ(matrix(1, 0), factor * 0.25);
    EXPECT_DOUBLE_EQ(matrix(1, 1), factor * (1.0 - 0.25));
    EXPECT_DOUBLE_EQ(matrix(2, 2), factor * (1.0 - 2.0 * 0.25) / 2.0);
}

TEST(PlaneStrainConstitutiveMatrix, HasZeroNormalShearCoupling)
{
    const IsotropicElasticMaterial material(1, 210.0e9, 0.3);

    const auto matrix = plane_strain_constitutive_matrix(material);

    EXPECT_DOUBLE_EQ(matrix(0, 2), 0.0);
    EXPECT_DOUBLE_EQ(matrix(1, 2), 0.0);
    EXPECT_DOUBLE_EQ(matrix(2, 0), 0.0);
    EXPECT_DOUBLE_EQ(matrix(2, 1), 0.0);
}

TEST(PlaneStrainConstitutiveMatrix, ProducesConfinementStressForZeroTransverseStrain)
{
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    const auto matrix = plane_strain_constitutive_matrix(material);
    DenseVector strain(3);
    strain[0] = 0.01;

    const DenseVector stress = matrix * strain;

    EXPECT_NEAR(stress[0], 2.4, 1.0e-12);
    EXPECT_NEAR(stress[1], 0.8, 1.0e-12);
    EXPECT_NEAR(stress[2], 0.0, 1.0e-12);
}

TEST(PlaneStrainConstitutiveMatrix, ProducesCorrectPureShearStress)
{
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    const auto matrix = plane_strain_constitutive_matrix(material);
    DenseVector strain(3);
    strain[2] = 0.01;

    const DenseVector stress = matrix * strain;

    const double shear_modulus = 200.0 / (2.0 * (1.0 + 0.25));
    EXPECT_NEAR(stress[0], 0.0, 1.0e-12);
    EXPECT_NEAR(stress[1], 0.0, 1.0e-12);
    EXPECT_NEAR(stress[2], shear_modulus * 0.01, 1.0e-12);
}

TEST(SolidIsotropicConstitutiveMatrix, ReturnsExpectedSixBySixMatrix)
{
    const IsotropicElasticMaterial material(1, 100.0, 0.25);

    const auto matrix = solid_isotropic_constitutive_matrix(material);

    ASSERT_EQ(matrix.rows(), 6);
    ASSERT_EQ(matrix.columns(), 6);
    const double shear_modulus = 40.0;
    const double lame_first_parameter = 40.0;
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            EXPECT_DOUBLE_EQ(matrix(row, column), row == column
                                                      ? lame_first_parameter + 2.0 * shear_modulus
                                                      : lame_first_parameter);
        }
    }
    EXPECT_DOUBLE_EQ(matrix(3, 3), shear_modulus);
    EXPECT_DOUBLE_EQ(matrix(4, 4), shear_modulus);
    EXPECT_DOUBLE_EQ(matrix(5, 5), shear_modulus);
    EXPECT_DOUBLE_EQ(matrix(0, 3), 0.0);
    EXPECT_DOUBLE_EQ(matrix(3, 0), 0.0);
    EXPECT_DOUBLE_EQ(matrix(3, 4), 0.0);
}

TEST(SolidIsotropicConstitutiveMatrix, ConvertsCompatibleStrainToUniaxialStress)
{
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    const auto matrix = solid_isotropic_constitutive_matrix(material);
    DenseVector strain(6);
    strain[0] = 0.01;
    strain[1] = -0.25 * 0.01;
    strain[2] = -0.25 * 0.01;

    const DenseVector stress = matrix * strain;

    EXPECT_NEAR(stress[0], 2.0, 1.0e-12);
    EXPECT_NEAR(stress[1], 0.0, 1.0e-12);
    EXPECT_NEAR(stress[2], 0.0, 1.0e-12);
}

TEST(SolidIsotropicConstitutiveMatrix, ProducesHydrostaticStress)
{
    const IsotropicElasticMaterial material(1, 150.0, 0.25);
    const auto matrix = solid_isotropic_constitutive_matrix(material);
    DenseVector strain(6);
    strain[0] = 0.01;
    strain[1] = 0.01;
    strain[2] = 0.01;

    const DenseVector stress = matrix * strain;

    const double expected_stress = 150.0 / (1.0 - 2.0 * 0.25) * 0.01;
    EXPECT_NEAR(stress[0], expected_stress, 1.0e-12);
    EXPECT_NEAR(stress[1], expected_stress, 1.0e-12);
    EXPECT_NEAR(stress[2], expected_stress, 1.0e-12);
}

TEST(SolidIsotropicConstitutiveMatrix, ProducesIndependentEngineeringShearStresses)
{
    const IsotropicElasticMaterial material(1, 200.0, 0.25);
    const auto matrix = solid_isotropic_constitutive_matrix(material);
    DenseVector strain(6);
    strain[3] = 0.01;
    strain[4] = -0.02;
    strain[5] = 0.03;

    const DenseVector stress = matrix * strain;

    const double shear_modulus = 200.0 / (2.0 * (1.0 + 0.25));
    EXPECT_NEAR(stress[3], shear_modulus * 0.01, 1.0e-12);
    EXPECT_NEAR(stress[4], shear_modulus * -0.02, 1.0e-12);
    EXPECT_NEAR(stress[5], shear_modulus * 0.03, 1.0e-12);
}
} // namespace
