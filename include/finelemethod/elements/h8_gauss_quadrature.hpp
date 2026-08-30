#pragma once

#include <array>

namespace finelemethod::elements
{
struct H8GaussPoint
{
    double xi;
    double eta;
    double zeta;
    double weight;
};

using H8GaussQuadrature = std::array<H8GaussPoint, 8>;

[[nodiscard]] H8GaussQuadrature h8_gauss_quadrature_2x2x2();
} // namespace finelemethod::elements
