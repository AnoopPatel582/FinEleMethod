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
    // Solver pipeline: impose essential boundary conditions in COO form,
    // compress to CSR, then solve the symmetric system with Conjugate Gradient.
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

    // Reactions must use the original, unconstrained equilibrium equation
    // R = K_original*u - F_original.
    math::DenseVector reactions =
        math::convert_to_csr(stiffness_matrix) * iterative_solution.solution - load_vector;
    return SparseStaticSolution{std::move(iterative_solution.solution), std::move(reactions),
                                iterative_solution.iterations, iterative_solution.residual_norm};
}
} // namespace finelemethod::solver
