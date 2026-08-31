#pragma once

#include "finelemethod/elements/h8_jacobian.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/h8_face_pressure_load.hpp"

namespace finelemethod::elements
{
// Returns the 24 consistent equivalent nodal forces for uniform pressure on an
// H8 face. Positive pressure acts inward toward the element centroid.
[[nodiscard]] math::DenseVector h8_uniform_face_pressure_load(const H8NodeCoordinates &coordinates,
                                                              model::H8Face face, double pressure);
} // namespace finelemethod::elements
