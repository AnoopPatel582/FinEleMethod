#pragma once

#include "finelemethod/math/coo_matrix.hpp"
#include "finelemethod/math/dense_matrix.hpp"
#include "finelemethod/math/dense_vector.hpp"

#include <cstddef>
#include <span>

namespace finelemethod::solver
{
struct PrescribedDisplacement
{
    std::size_t degree_of_freedom;
    double value;
};

struct SparseConstrainedSystem
{
    math::CooMatrix stiffness_matrix;
    math::DenseVector load_vector;
};

void apply_prescribed_displacement(math::DenseMatrix &stiffness_matrix,
                                   math::DenseVector &load_vector, std::size_t degree_of_freedom,
                                   double prescribed_value);

void apply_prescribed_displacements(
    math::DenseMatrix &stiffness_matrix, math::DenseVector &load_vector,
    std::span<const PrescribedDisplacement> prescribed_displacements);

// Applies direct elimination to a COO system without modifying its inputs. The
// resulting matrix remains symmetric when the original matrix is symmetric.
[[nodiscard]] SparseConstrainedSystem apply_prescribed_displacements(
    const math::CooMatrix &stiffness_matrix, const math::DenseVector &load_vector,
    std::span<const PrescribedDisplacement> prescribed_displacements);
} // namespace finelemethod::solver
