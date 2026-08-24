#include "finelemethod/math/linear_solver.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace finelemethod::math
{
namespace
{
double matrix_scale(const DenseMatrix &matrix)
{
    double scale = 0.0;
    for (DenseMatrix::size_type row = 0; row < matrix.rows(); ++row)
    {
        for (DenseMatrix::size_type column = 0; column < matrix.columns(); ++column)
        {
            scale = std::max(scale, std::abs(matrix(row, column)));
        }
    }

    return scale;
}

void validate_system_dimensions(const DenseMatrix &matrix, const DenseVector &right_hand_side)
{
    if (matrix.rows() != matrix.columns())
    {
        throw std::invalid_argument("Gaussian elimination requires a square matrix.");
    }

    if (matrix.rows() != right_hand_side.size())
    {
        throw std::invalid_argument(
            "Matrix row count must match right-hand-side size for Gaussian elimination.");
    }
}
} // namespace

DenseVector solve_gaussian_elimination(DenseMatrix matrix, DenseVector right_hand_side)
{
    validate_system_dimensions(matrix, right_hand_side);

    const DenseMatrix::size_type system_size = matrix.rows();
    const double scale = matrix_scale(matrix);
    const double pivot_tolerance =
        std::numeric_limits<double>::epsilon() * static_cast<double>(system_size) * scale;

    for (DenseMatrix::size_type pivot_column = 0; pivot_column < system_size; ++pivot_column)
    {
        DenseMatrix::size_type pivot_row = pivot_column;
        double largest_pivot = std::abs(matrix(pivot_column, pivot_column));

        for (DenseMatrix::size_type row = pivot_column + 1; row < system_size; ++row)
        {
            const double candidate = std::abs(matrix(row, pivot_column));
            if (candidate > largest_pivot)
            {
                largest_pivot = candidate;
                pivot_row = row;
            }
        }

        if (largest_pivot <= pivot_tolerance)
        {
            throw std::runtime_error("Gaussian elimination cannot solve a singular matrix.");
        }

        if (pivot_row != pivot_column)
        {
            for (DenseMatrix::size_type column = 0; column < system_size; ++column)
            {
                std::swap(matrix(pivot_column, column), matrix(pivot_row, column));
            }
            std::swap(right_hand_side[pivot_column], right_hand_side[pivot_row]);
        }

        for (DenseMatrix::size_type row = pivot_column + 1; row < system_size; ++row)
        {
            const double factor = matrix(row, pivot_column) / matrix(pivot_column, pivot_column);
            matrix(row, pivot_column) = 0.0;

            for (DenseMatrix::size_type column = pivot_column + 1; column < system_size; ++column)
            {
                matrix(row, column) -= factor * matrix(pivot_column, column);
            }
            right_hand_side[row] -= factor * right_hand_side[pivot_column];
        }
    }

    DenseVector solution(system_size);
    for (DenseMatrix::size_type row = system_size; row-- > 0;)
    {
        double value = right_hand_side[row];
        for (DenseMatrix::size_type column = row + 1; column < system_size; ++column)
        {
            value -= matrix(row, column) * solution[column];
        }

        solution[row] = value / matrix(row, row);
    }

    return solution;
}
} // namespace finelemethod::math
