#include "finelemethod/elements/h8_face_pressure.hpp"

#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::elements::h8_uniform_face_pressure_load;
using finelemethod::elements::H8NodeCoordinates;
using finelemethod::model::H8Face;

constexpr H8NodeCoordinates box{{{{0.0, 0.0, 0.0}},
                                 {{2.0, 0.0, 0.0}},
                                 {{2.0, 3.0, 0.0}},
                                 {{0.0, 3.0, 0.0}},
                                 {{0.0, 0.0, 4.0}},
                                 {{2.0, 0.0, 4.0}},
                                 {{2.0, 3.0, 4.0}},
                                 {{0.0, 3.0, 4.0}}}};

std::array<double, 3> total_force(const finelemethod::math::DenseVector &load)
{
    std::array<double, 3> total{};
    for (std::size_t node = 0; node < 8; ++node)
    {
        for (std::size_t component = 0; component < 3; ++component)
        {
            total[component] += load[3 * node + component];
        }
    }
    return total;
}

TEST(H8FacePressure, DistributesUniformPressureConsistentlyToFaceNodes)
{
    const auto load = h8_uniform_face_pressure_load(box, H8Face::two, 10.0);

    ASSERT_EQ(load.size(), 24U);
    for (const std::size_t node : {4U, 5U, 6U, 7U})
    {
        EXPECT_NEAR(load[3 * node + 2], -15.0, 1.0e-12);
    }
    EXPECT_NEAR(total_force(load)[2], -60.0, 1.0e-12);
}

TEST(H8FacePressure, UsesInwardNormalsForAllAbaqusFaces)
{
    EXPECT_NEAR(total_force(h8_uniform_face_pressure_load(box, H8Face::one, 1.0))[2], 6.0, 1.0e-12);
    EXPECT_NEAR(total_force(h8_uniform_face_pressure_load(box, H8Face::two, 1.0))[2], -6.0,
                1.0e-12);
    EXPECT_NEAR(total_force(h8_uniform_face_pressure_load(box, H8Face::three, 1.0))[1], 8.0,
                1.0e-12);
    EXPECT_NEAR(total_force(h8_uniform_face_pressure_load(box, H8Face::four, 1.0))[0], -12.0,
                1.0e-12);
    EXPECT_NEAR(total_force(h8_uniform_face_pressure_load(box, H8Face::five, 1.0))[1], -8.0,
                1.0e-12);
    EXPECT_NEAR(total_force(h8_uniform_face_pressure_load(box, H8Face::six, 1.0))[0], 12.0,
                1.0e-12);
}

TEST(H8FacePressure, RejectsInvalidInputs)
{
    H8NodeCoordinates zero_face = box;
    zero_face[5] = zero_face[4];
    zero_face[6] = zero_face[7];
    const double infinity = std::numeric_limits<double>::infinity();

    EXPECT_THROW(static_cast<void>(h8_uniform_face_pressure_load(box, H8Face::one, infinity)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(h8_uniform_face_pressure_load(box, static_cast<H8Face>(6), 1.0)),
                 std::invalid_argument);
    EXPECT_THROW(static_cast<void>(h8_uniform_face_pressure_load(zero_face, H8Face::two, 1.0)),
                 std::invalid_argument);
}
} // namespace
