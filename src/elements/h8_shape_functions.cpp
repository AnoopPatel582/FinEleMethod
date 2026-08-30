#include "finelemethod/elements/h8_shape_functions.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::elements
{
H8ShapeFunctionValues h8_shape_functions(const double xi, const double eta, const double zeta)
{
    if (!std::isfinite(xi) || !std::isfinite(eta) || !std::isfinite(zeta))
    {
        throw std::invalid_argument("H8 natural coordinates must be finite.");
    }

    constexpr double factor = 0.125;
    return {
        factor * (1.0 - xi) * (1.0 - eta) * (1.0 - zeta),
        factor * (1.0 + xi) * (1.0 - eta) * (1.0 - zeta),
        factor * (1.0 + xi) * (1.0 + eta) * (1.0 - zeta),
        factor * (1.0 - xi) * (1.0 + eta) * (1.0 - zeta),
        factor * (1.0 - xi) * (1.0 - eta) * (1.0 + zeta),
        factor * (1.0 + xi) * (1.0 - eta) * (1.0 + zeta),
        factor * (1.0 + xi) * (1.0 + eta) * (1.0 + zeta),
        factor * (1.0 - xi) * (1.0 + eta) * (1.0 + zeta),
    };
}
} // namespace finelemethod::elements
