#pragma once

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

void apply_prescribed_displacement(math::DenseMatrix &stiffness_matrix,
                                   math::DenseVector &load_vector, std::size_t degree_of_freedom,
                                   double prescribed_value);

void apply_prescribed_displacements(
    math::DenseMatrix &stiffness_matrix, math::DenseVector &load_vector,
    std::span<const PrescribedDisplacement> prescribed_displacements);
} // namespace finelemethod::solver
