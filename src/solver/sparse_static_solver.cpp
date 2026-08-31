#include "finelemethod/solver/sparse_static_solver.hpp"

#include "finelemethod/math/csr_matrix.hpp"

#include <stdexcept>
#include <utility>

namespace finelemethod::solver
{
SparseStaticSolution solve_sparse_static_system(
    const math::CooMatrix &stiffness_matrix, const math::DenseVector &load_vector,
    const std::span<const PrescribedDisplacement> prescribed_displacements,
    const ConjugateGradientOptions &options)
{
    const SparseConstrainedSystem constrained =
        apply_prescribed_displacements(stiffness_matrix, load_vector, prescribed_displacements);
    const math::CsrMatrix constrained_stiffness =
        math::convert_to_csr(constrained.stiffness_matrix);
    ConjugateGradientResult iterative_solution =
        solve_conjugate_gradient(constrained_stiffness, constrained.load_vector, options);
    if (!iterative_solution.converged)
    {
        throw std::runtime_error(
            "Sparse static system did not converge within the iteration limit.");
    }

    math::DenseVector reactions =
        math::convert_to_csr(stiffness_matrix) * iterative_solution.solution - load_vector;
    return SparseStaticSolution{std::move(iterative_solution.solution), std::move(reactions),
                                iterative_solution.iterations, iterative_solution.residual_norm};
}
} // namespace finelemethod::solver
