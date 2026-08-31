#include "finelemethod/model/h8_face_pressure_load.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using finelemethod::model::H8Face;
using finelemethod::model::H8FacePressureLoad;

TEST(H8FacePressureLoad, StoresElementFaceAndPressure)
{
    const H8FacePressureLoad load(12, H8Face::five, -25.5);

    EXPECT_EQ(load.element_id(), 12U);
    EXPECT_EQ(load.face(), H8Face::five);
    EXPECT_DOUBLE_EQ(load.pressure(), -25.5);
}

TEST(H8FacePressureLoad, RejectsInvalidFaceAndNonfinitePressure)
{
    const double infinity = std::numeric_limits<double>::infinity();

    EXPECT_THROW(H8FacePressureLoad(1, static_cast<H8Face>(6), 1.0), std::invalid_argument);
    EXPECT_THROW(H8FacePressureLoad(1, H8Face::one, infinity), std::invalid_argument);
}
} // namespace
