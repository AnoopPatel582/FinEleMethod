#pragma once

#include <array>

namespace finelemethod::elements
{
struct Q4GaussPoint
{
    double xi;
    double eta;
    double weight;
};

using Q4GaussQuadrature = std::array<Q4GaussPoint, 4>;

[[nodiscard]] Q4GaussQuadrature q4_gauss_quadrature_2x2();
} // namespace finelemethod::elements
