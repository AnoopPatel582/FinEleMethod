#include "finelemethod/elements/h8_gauss_quadrature.hpp"

#include <cmath>

namespace finelemethod::elements
{
H8GaussQuadrature h8_gauss_quadrature_2x2x2()
{
    const double coordinate = 1.0 / std::sqrt(3.0);

    return {{{-coordinate, -coordinate, -coordinate, 1.0},
             {coordinate, -coordinate, -coordinate, 1.0},
             {coordinate, coordinate, -coordinate, 1.0},
             {-coordinate, coordinate, -coordinate, 1.0},
             {-coordinate, -coordinate, coordinate, 1.0},
             {coordinate, -coordinate, coordinate, 1.0},
             {coordinate, coordinate, coordinate, 1.0},
             {-coordinate, coordinate, coordinate, 1.0}}};
}
} // namespace finelemethod::elements
