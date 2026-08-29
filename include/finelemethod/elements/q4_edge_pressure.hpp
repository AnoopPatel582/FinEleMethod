#pragma once

#include "finelemethod/elements/q4_jacobian.hpp"
#include "finelemethod/math/dense_vector.hpp"

#include <cstddef>

namespace finelemethod::elements
{
enum class Q4Edge : std::size_t
{
    one,
    two,
    three,
    four
};

// Returns the eight equivalent nodal forces for a uniform pressure on one edge.
// The Q4 nodes must be counterclockwise. Positive pressure acts inward.
[[nodiscard]] math::DenseVector q4_uniform_edge_pressure_load(const Q4NodeCoordinates &coordinates,
                                                              Q4Edge edge, double pressure,
                                                              double thickness);
} // namespace finelemethod::elements
