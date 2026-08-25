#include "finelemethod/solver/dense_static_solver.hpp"

#include "finelemethod/math/linear_solver.hpp"
#include "finelemethod/solver/reactions.hpp"

#include <utility>

namespace finelemethod::solver
{
DenseStaticResult solve_dense_static_system(
    const math::DenseMatrix &stiffness_matrix, const math::DenseVector &load_vector,
    const std::span<const PrescribedDisplacement> prescribed_displacements)
{
    math::DenseMatrix constrained_stiffness_matrix = stiffness_matrix;
    math::DenseVector constrained_load_vector = load_vector;

    apply_prescribed_displacements(constrained_stiffness_matrix, constrained_load_vector,
                                   prescribed_displacements);

    math::DenseVector displacements =
        math::solve_gaussian_elimination(constrained_stiffness_matrix, constrained_load_vector);
    math::DenseVector reactions =
        calculate_reaction_forces(stiffness_matrix, load_vector, displacements);

    return DenseStaticResult{std::move(displacements), std::move(reactions)};
}
} // namespace finelemethod::solver
