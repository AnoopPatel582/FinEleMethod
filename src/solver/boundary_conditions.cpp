#include "finelemethod/solver/boundary_conditions.hpp"

#include <stdexcept>
#include <vector>

namespace finelemethod::solver
{
namespace
{
void validate_system(const math::DenseMatrix &stiffness_matrix,
                     const math::DenseVector &load_vector)
{
    if (stiffness_matrix.rows() != stiffness_matrix.columns())
    {
        throw std::invalid_argument("Prescribed displacement requires a square stiffness matrix.");
    }

    if (stiffness_matrix.rows() != load_vector.size())
    {
        throw std::invalid_argument("Stiffness matrix row count must match load-vector size.");
    }
}

void validate_degree_of_freedom(const math::DenseVector &load_vector,
                                const std::size_t degree_of_freedom)
{
    if (degree_of_freedom >= load_vector.size())
    {
        throw std::out_of_range("Prescribed displacement degree of freedom is out of range.");
    }
}

void apply_prescribed_displacement_unchecked(math::DenseMatrix &stiffness_matrix,
                                             math::DenseVector &load_vector,
                                             const std::size_t degree_of_freedom,
                                             const double prescribed_value)
{
    for (math::DenseMatrix::size_type row = 0; row < stiffness_matrix.rows(); ++row)
    {
        if (row == degree_of_freedom)
        {
            continue;
        }

        load_vector[row] -= stiffness_matrix(row, degree_of_freedom) * prescribed_value;
        stiffness_matrix(row, degree_of_freedom) = 0.0;
    }

    for (math::DenseMatrix::size_type column = 0; column < stiffness_matrix.columns(); ++column)
    {
        stiffness_matrix(degree_of_freedom, column) = 0.0;
    }

    stiffness_matrix(degree_of_freedom, degree_of_freedom) = 1.0;
    load_vector[degree_of_freedom] = prescribed_value;
}
} // namespace

void apply_prescribed_displacement(math::DenseMatrix &stiffness_matrix,
                                   math::DenseVector &load_vector,
                                   const std::size_t degree_of_freedom,
                                   const double prescribed_value)
{
    validate_system(stiffness_matrix, load_vector);
    validate_degree_of_freedom(load_vector, degree_of_freedom);
    apply_prescribed_displacement_unchecked(stiffness_matrix, load_vector, degree_of_freedom,
                                            prescribed_value);
}

void apply_prescribed_displacements(
    math::DenseMatrix &stiffness_matrix, math::DenseVector &load_vector,
    const std::span<const PrescribedDisplacement> prescribed_displacements)
{
    validate_system(stiffness_matrix, load_vector);

    std::vector<bool> constrained_degrees_of_freedom(load_vector.size(), false);
    for (const PrescribedDisplacement &prescribed_displacement : prescribed_displacements)
    {
        validate_degree_of_freedom(load_vector, prescribed_displacement.degree_of_freedom);

        if (constrained_degrees_of_freedom[prescribed_displacement.degree_of_freedom])
        {
            throw std::invalid_argument(
                "Each prescribed displacement degree of freedom must be unique.");
        }

        constrained_degrees_of_freedom[prescribed_displacement.degree_of_freedom] = true;
    }

    for (const PrescribedDisplacement &prescribed_displacement : prescribed_displacements)
    {
        apply_prescribed_displacement_unchecked(stiffness_matrix, load_vector,
                                                prescribed_displacement.degree_of_freedom,
                                                prescribed_displacement.value);
    }
}
} // namespace finelemethod::solver
