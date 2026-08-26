#include "finelemethod/model/material_collection.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{
using finelemethod::model::IsotropicElasticMaterial;
using finelemethod::model::MaterialCollection;

TEST(MaterialCollection, StartsEmpty)
{
    const MaterialCollection materials;

    EXPECT_TRUE(materials.empty());
    EXPECT_EQ(materials.size(), 0);
    EXPECT_TRUE(materials.materials().empty());
}

TEST(MaterialCollection, AddsMaterialsAndFindsThemById)
{
    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(10, 210.0e9, 0.3));
    materials.add(IsotropicElasticMaterial(25, 70.0e9, 0.33));

    EXPECT_FALSE(materials.empty());
    EXPECT_EQ(materials.size(), 2);
    EXPECT_TRUE(materials.contains(10));
    EXPECT_TRUE(materials.contains(25));
    EXPECT_FALSE(materials.contains(99));
    EXPECT_DOUBLE_EQ(materials.at(10).youngs_modulus(), 210.0e9);
    EXPECT_DOUBLE_EQ(materials.at(25).poisson_ratio(), 0.33);
}

TEST(MaterialCollection, PreservesInsertionOrder)
{
    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(30, 30.0, 0.3));
    materials.add(IsotropicElasticMaterial(10, 10.0, 0.2));
    materials.add(IsotropicElasticMaterial(20, 20.0, 0.25));

    const auto ordered_materials = materials.materials();

    ASSERT_EQ(ordered_materials.size(), 3);
    EXPECT_EQ(ordered_materials[0].id(), 30);
    EXPECT_EQ(ordered_materials[1].id(), 10);
    EXPECT_EQ(ordered_materials[2].id(), 20);
}

TEST(MaterialCollection, RejectsDuplicateIdWithoutChangingCollection)
{
    MaterialCollection materials;
    materials.add(IsotropicElasticMaterial(10, 210.0e9, 0.3));

    EXPECT_THROW(materials.add(IsotropicElasticMaterial(10, 70.0e9, 0.33)), std::invalid_argument);
    EXPECT_EQ(materials.size(), 1);
    EXPECT_DOUBLE_EQ(materials.at(10).youngs_modulus(), 210.0e9);
    EXPECT_DOUBLE_EQ(materials.at(10).poisson_ratio(), 0.3);
}

TEST(MaterialCollection, RejectsMissingIdLookup)
{
    const MaterialCollection materials;

    EXPECT_THROW(static_cast<void>(materials.at(123)), std::out_of_range);
}
} // namespace
