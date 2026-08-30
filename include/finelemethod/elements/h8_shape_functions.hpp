#pragma once

#include <array>

namespace finelemethod::elements
{
using H8ShapeFunctionValues = std::array<double, 8>;

// Natural node order:
// 1 (-1,-1,-1), 2 (1,-1,-1), 3 (1,1,-1), 4 (-1,1,-1),
// 5 (-1,-1, 1), 6 (1,-1, 1), 7 (1,1, 1), 8 (-1,1, 1).
[[nodiscard]] H8ShapeFunctionValues h8_shape_functions(double xi, double eta, double zeta);
} // namespace finelemethod::elements
