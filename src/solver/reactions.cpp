#include "finelemethod/solver/reactions.hpp"

#include <stdexcept>

namespace finelemethod::solver
{
math::DenseVector calculate_reaction_forces(const math::DenseMatrix &original_stiffness_matrix,
                                            const math::DenseVector &original_load_vector,
                                            const math::DenseVector &displacement_vector)
{
    if (original_stiffness_matrix.rows() != original_stiffness_matrix.columns())
    {
        throw std::invalid_argument("Reaction calculation requires a square stiffness matrix.");
    }

    if (original_stiffness_matrix.rows() != original_load_vector.size())
    {
        throw std::invalid_argument(
            "Stiffness matrix row count must match original load-vector size.");
    }

    if (original_stiffness_matrix.columns() != displacement_vector.size())
    {
        throw std::invalid_argument(
            "Stiffness matrix column count must match displacement-vector size.");
    }

    return original_stiffness_matrix * displacement_vector - original_load_vector;
}
} // namespace finelemethod::solver
