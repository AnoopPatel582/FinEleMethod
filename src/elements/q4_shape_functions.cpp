#include "finelemethod/elements/q4_shape_functions.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::elements
{
namespace
{
void validate_natural_coordinates(const double xi, const double eta)
{
    if (!std::isfinite(xi) || !std::isfinite(eta))
    {
        throw std::invalid_argument("Q4 natural coordinates must be finite.");
    }
}
} // namespace

Q4ShapeFunctionValues q4_shape_functions(const double xi, const double eta)
{
    validate_natural_coordinates(xi, eta);

    return {0.25 * (1.0 - xi) * (1.0 - eta), 0.25 * (1.0 + xi) * (1.0 - eta),
            0.25 * (1.0 + xi) * (1.0 + eta), 0.25 * (1.0 - xi) * (1.0 + eta)};
}

math::DenseMatrix q4_shape_function_natural_derivatives(const double xi, const double eta)
{
    validate_natural_coordinates(xi, eta);

    math::DenseMatrix derivatives(2, 4);

    derivatives(0, 0) = -0.25 * (1.0 - eta);
    derivatives(0, 1) = 0.25 * (1.0 - eta);
    derivatives(0, 2) = 0.25 * (1.0 + eta);
    derivatives(0, 3) = -0.25 * (1.0 + eta);

    derivatives(1, 0) = -0.25 * (1.0 - xi);
    derivatives(1, 1) = -0.25 * (1.0 + xi);
    derivatives(1, 2) = 0.25 * (1.0 + xi);
    derivatives(1, 3) = 0.25 * (1.0 - xi);

    return derivatives;
}
} // namespace finelemethod::elements
