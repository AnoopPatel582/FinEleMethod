#pragma once

#include "finelemethod/math/dense_matrix.hpp"

#include <array>

namespace finelemethod::elements
{
using H8ShapeFunctionValues = std::array<double, 8>;

// Natural node order:
// 1 (-1,-1,-1), 2 (1,-1,-1), 3 (1,1,-1), 4 (-1,1,-1),
// 5 (-1,-1, 1), 6 (1,-1, 1), 7 (1,1, 1), 8 (-1,1, 1).
[[nodiscard]] H8ShapeFunctionValues h8_shape_functions(double xi, double eta, double zeta);

// Returns a 3-by-8 matrix. Rows contain dN/dxi, dN/deta, and dN/dzeta.
[[nodiscard]] math::DenseMatrix h8_shape_function_natural_derivatives(double xi, double eta,
                                                                      double zeta);
} // namespace finelemethod::elements
