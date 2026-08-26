#include "finelemethod/elements/q4_gauss_quadrature.hpp"

#include <cmath>

namespace finelemethod::elements
{
Q4GaussQuadrature q4_gauss_quadrature_2x2()
{
    const double coordinate = 1.0 / std::sqrt(3.0);

    return {{{-coordinate, -coordinate, 1.0},
             {coordinate, -coordinate, 1.0},
             {coordinate, coordinate, 1.0},
             {-coordinate, coordinate, 1.0}}};
}
} // namespace finelemethod::elements
