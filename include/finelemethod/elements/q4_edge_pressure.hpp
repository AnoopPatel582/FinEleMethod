#pragma once

#include "finelemethod/elements/q4_jacobian.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/q4_edge_pressure_load.hpp"

namespace finelemethod::elements
{
// Returns the eight equivalent nodal forces for a uniform pressure on one edge.
// The Q4 nodes must be counterclockwise. Positive pressure acts inward.
[[nodiscard]] math::DenseVector q4_uniform_edge_pressure_load(const Q4NodeCoordinates &coordinates,
                                                              model::Q4Edge edge, double pressure,
                                                              double thickness);
} // namespace finelemethod::elements
