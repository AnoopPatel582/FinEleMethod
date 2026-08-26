#include "finelemethod/elements/q4_strain_displacement.hpp"

#include "finelemethod/elements/q4_physical_derivatives.hpp"

#include <cstddef>
#include <utility>

namespace finelemethod::elements
{
Q4StrainDisplacement q4_strain_displacement_matrix(const Q4NodeCoordinates &coordinates,
                                                   const double xi, const double eta)
{
    const Q4PhysicalDerivatives physical =
        q4_shape_function_physical_derivatives(coordinates, xi, eta);
    math::DenseMatrix matrix(3, 8);

    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const std::size_t x_dof = 2 * node;
        const std::size_t y_dof = x_dof + 1;
        const double derivative_x = physical.derivatives(0, node);
        const double derivative_y = physical.derivatives(1, node);

        matrix(0, x_dof) = derivative_x;
        matrix(1, y_dof) = derivative_y;
        matrix(2, x_dof) = derivative_y;
        matrix(2, y_dof) = derivative_x;
    }

    return {std::move(matrix), physical.jacobian_determinant};
}
} // namespace finelemethod::elements
