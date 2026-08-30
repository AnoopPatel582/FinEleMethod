#include "finelemethod/elements/h8_strain_displacement.hpp"

#include "finelemethod/elements/h8_physical_derivatives.hpp"

#include <cstddef>
#include <utility>

namespace finelemethod::elements
{
H8StrainDisplacement h8_strain_displacement_matrix(const H8NodeCoordinates &coordinates,
                                                   const double xi, const double eta,
                                                   const double zeta)
{
    const H8PhysicalDerivatives physical =
        h8_shape_function_physical_derivatives(coordinates, xi, eta, zeta);
    math::DenseMatrix matrix(6, 24);

    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const std::size_t x_dof = 3 * node;
        const std::size_t y_dof = x_dof + 1;
        const std::size_t z_dof = x_dof + 2;
        const double derivative_x = physical.derivatives(0, node);
        const double derivative_y = physical.derivatives(1, node);
        const double derivative_z = physical.derivatives(2, node);

        matrix(0, x_dof) = derivative_x;
        matrix(1, y_dof) = derivative_y;
        matrix(2, z_dof) = derivative_z;
        matrix(3, x_dof) = derivative_y;
        matrix(3, y_dof) = derivative_x;
        matrix(4, y_dof) = derivative_z;
        matrix(4, z_dof) = derivative_y;
        matrix(5, x_dof) = derivative_z;
        matrix(5, z_dof) = derivative_x;
    }

    return {std::move(matrix), physical.jacobian_determinant};
}
} // namespace finelemethod::elements
