#include "finelemethod/elements/h8_shape_functions.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace finelemethod::elements
{
namespace
{
void validate_natural_coordinates(const double xi, const double eta, const double zeta)
{
    if (!std::isfinite(xi) || !std::isfinite(eta) || !std::isfinite(zeta))
    {
        throw std::invalid_argument("H8 natural coordinates must be finite.");
    }
}

constexpr std::array<std::array<double, 3>, 8> natural_node_signs{{
    {{-1.0, -1.0, -1.0}},
    {{1.0, -1.0, -1.0}},
    {{1.0, 1.0, -1.0}},
    {{-1.0, 1.0, -1.0}},
    {{-1.0, -1.0, 1.0}},
    {{1.0, -1.0, 1.0}},
    {{1.0, 1.0, 1.0}},
    {{-1.0, 1.0, 1.0}},
}};
} // namespace

H8ShapeFunctionValues h8_shape_functions(const double xi, const double eta, const double zeta)
{
    validate_natural_coordinates(xi, eta, zeta);

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

math::DenseMatrix h8_shape_function_natural_derivatives(const double xi, const double eta,
                                                        const double zeta)
{
    validate_natural_coordinates(xi, eta, zeta);

    math::DenseMatrix derivatives(3, 8);
    constexpr double factor = 0.125;
    for (std::size_t node = 0; node < natural_node_signs.size(); ++node)
    {
        const double xi_sign = natural_node_signs[node][0];
        const double eta_sign = natural_node_signs[node][1];
        const double zeta_sign = natural_node_signs[node][2];
        derivatives(0, node) = factor * xi_sign * (1.0 + eta_sign * eta) * (1.0 + zeta_sign * zeta);
        derivatives(1, node) = factor * eta_sign * (1.0 + xi_sign * xi) * (1.0 + zeta_sign * zeta);
        derivatives(2, node) = factor * zeta_sign * (1.0 + xi_sign * xi) * (1.0 + eta_sign * eta);
    }
    return derivatives;
}
} // namespace finelemethod::elements
