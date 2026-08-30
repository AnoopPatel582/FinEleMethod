#include "finelemethod/elements/h8_jacobian.hpp"

#include "finelemethod/elements/h8_shape_functions.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

namespace finelemethod::elements
{
H8Jacobian h8_jacobian(const H8NodeCoordinates &coordinates, const double xi, const double eta,
                       const double zeta)
{
    for (const auto &coordinate : coordinates)
    {
        if (!std::isfinite(coordinate[0]) || !std::isfinite(coordinate[1]) ||
            !std::isfinite(coordinate[2]))
        {
            throw std::invalid_argument("H8 nodal coordinates must be finite.");
        }
    }

    const math::DenseMatrix natural_derivatives =
        h8_shape_function_natural_derivatives(xi, eta, zeta);
    math::DenseMatrix jacobian(3, 3);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        for (std::size_t natural_coordinate = 0; natural_coordinate < 3; ++natural_coordinate)
        {
            for (std::size_t physical_coordinate = 0; physical_coordinate < 3;
                 ++physical_coordinate)
            {
                jacobian(natural_coordinate, physical_coordinate) +=
                    natural_derivatives(natural_coordinate, node) *
                    coordinates[node][physical_coordinate];
            }
        }
    }

    const double a = jacobian(0, 0);
    const double b = jacobian(0, 1);
    const double c = jacobian(0, 2);
    const double d = jacobian(1, 0);
    const double e = jacobian(1, 1);
    const double f = jacobian(1, 2);
    const double g = jacobian(2, 0);
    const double h = jacobian(2, 1);
    const double i = jacobian(2, 2);
    const double determinant = a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
    const double scale = std::max({std::abs(a), std::abs(b), std::abs(c), std::abs(d), std::abs(e),
                                   std::abs(f), std::abs(g), std::abs(h), std::abs(i)});
    const double determinant_tolerance =
        64.0 * std::numeric_limits<double>::epsilon() * scale * scale * scale;
    if (!std::isfinite(determinant) || determinant <= determinant_tolerance)
    {
        throw std::invalid_argument("H8 Jacobian determinant must be positive and non-degenerate.");
    }

    math::DenseMatrix inverse(3, 3);
    inverse(0, 0) = (e * i - f * h) / determinant;
    inverse(0, 1) = (c * h - b * i) / determinant;
    inverse(0, 2) = (b * f - c * e) / determinant;
    inverse(1, 0) = (f * g - d * i) / determinant;
    inverse(1, 1) = (a * i - c * g) / determinant;
    inverse(1, 2) = (c * d - a * f) / determinant;
    inverse(2, 0) = (d * h - e * g) / determinant;
    inverse(2, 1) = (b * g - a * h) / determinant;
    inverse(2, 2) = (a * e - b * d) / determinant;

    return {std::move(jacobian), determinant, std::move(inverse)};
}
} // namespace finelemethod::elements
