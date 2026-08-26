#pragma once

#include "finelemethod/math/dense_matrix.hpp"

#include <array>

namespace finelemethod::elements
{
using Q4ShapeFunctionValues = std::array<double, 4>;

// Natural node order: (-1,-1), (1,-1), (1,1), (-1,1).
[[nodiscard]] Q4ShapeFunctionValues q4_shape_functions(double xi, double eta);

// Returns a 2-by-4 matrix. Row 0 contains dN/dxi and row 1 contains dN/deta.
[[nodiscard]] math::DenseMatrix q4_shape_function_natural_derivatives(double xi, double eta);
} // namespace finelemethod::elements
