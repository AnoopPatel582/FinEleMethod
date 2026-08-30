#include "finelemethod/elements/h8_physical_derivatives.hpp"

#include "finelemethod/elements/h8_shape_functions.hpp"

#include <utility>

namespace finelemethod::elements
{
H8PhysicalDerivatives h8_shape_function_physical_derivatives(const H8NodeCoordinates &coordinates,
                                                             const double xi, const double eta,
                                                             const double zeta)
{
    const H8Jacobian jacobian = h8_jacobian(coordinates, xi, eta, zeta);
    const math::DenseMatrix natural_derivatives =
        h8_shape_function_natural_derivatives(xi, eta, zeta);
    math::DenseMatrix physical_derivatives = jacobian.inverse * natural_derivatives;

    return {std::move(physical_derivatives), jacobian.determinant};
}
} // namespace finelemethod::elements
