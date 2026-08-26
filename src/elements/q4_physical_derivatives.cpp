#include "finelemethod/elements/q4_physical_derivatives.hpp"

#include "finelemethod/elements/q4_shape_functions.hpp"

#include <utility>

namespace finelemethod::elements
{
Q4PhysicalDerivatives q4_shape_function_physical_derivatives(const Q4NodeCoordinates &coordinates,
                                                             const double xi, const double eta)
{
    const Q4Jacobian jacobian = q4_jacobian(coordinates, xi, eta);
    const math::DenseMatrix natural_derivatives = q4_shape_function_natural_derivatives(xi, eta);
    math::DenseMatrix physical_derivatives = jacobian.inverse * natural_derivatives;

    return {std::move(physical_derivatives), jacobian.determinant};
}
} // namespace finelemethod::elements
