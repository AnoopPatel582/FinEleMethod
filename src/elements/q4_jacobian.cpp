#include "finelemethod/elements/q4_jacobian.hpp"

#include "finelemethod/elements/q4_shape_functions.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

namespace finelemethod::elements
{
Q4Jacobian q4_jacobian(const Q4NodeCoordinates &coordinates, const double xi, const double eta)
{
    for (const auto &coordinate : coordinates)
    {
        if (!std::isfinite(coordinate[0]) || !std::isfinite(coordinate[1]))
        {
            throw std::invalid_argument("Q4 nodal coordinates must be finite.");
        }
    }

    const math::DenseMatrix natural_derivatives = q4_shape_function_natural_derivatives(xi, eta);

    math::DenseMatrix jacobian(2, 2);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        jacobian(0, 0) += natural_derivatives(0, node) * coordinates[node][0];
        jacobian(0, 1) += natural_derivatives(0, node) * coordinates[node][1];
        jacobian(1, 0) += natural_derivatives(1, node) * coordinates[node][0];
        jacobian(1, 1) += natural_derivatives(1, node) * coordinates[node][1];
    }

    const double determinant = jacobian(0, 0) * jacobian(1, 1) - jacobian(0, 1) * jacobian(1, 0);
    const double scale = std::max({std::abs(jacobian(0, 0)), std::abs(jacobian(0, 1)),
                                   std::abs(jacobian(1, 0)), std::abs(jacobian(1, 1))});
    const double determinant_tolerance =
        16.0 * std::numeric_limits<double>::epsilon() * scale * scale;

    if (!std::isfinite(determinant) || determinant <= determinant_tolerance)
    {
        throw std::invalid_argument("Q4 Jacobian determinant must be positive and non-degenerate.");
    }

    math::DenseMatrix inverse(2, 2);
    inverse(0, 0) = jacobian(1, 1) / determinant;
    inverse(0, 1) = -jacobian(0, 1) / determinant;
    inverse(1, 0) = -jacobian(1, 0) / determinant;
    inverse(1, 1) = jacobian(0, 0) / determinant;

    return {std::move(jacobian), determinant, std::move(inverse)};
}
} // namespace finelemethod::elements
