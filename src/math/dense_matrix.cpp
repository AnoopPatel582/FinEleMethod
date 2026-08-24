#include "finelemethod/math/dense_matrix.hpp"

#include <stdexcept>

namespace finelemethod::math
{
namespace
{
DenseMatrix::size_type checked_element_count(const DenseMatrix::size_type rows,
                                             const DenseMatrix::size_type columns)
{
    if (rows == 0 || columns == 0)
    {
        throw std::invalid_argument("DenseMatrix dimensions must be greater than zero.");
    }

    const auto maximum_size = std::vector<double>{}.max_size();
    if (rows > maximum_size / columns)
    {
        throw std::length_error("DenseMatrix dimensions are too large.");
    }

    return rows * columns;
}
} // namespace

DenseMatrix::DenseMatrix(const size_type rows, const size_type columns, const double initial_value)
    : rows_(rows), columns_(columns), values_(checked_element_count(rows, columns), initial_value)
{
}

DenseMatrix::size_type DenseMatrix::rows() const noexcept
{
    return rows_;
}

DenseMatrix::size_type DenseMatrix::columns() const noexcept
{
    return columns_;
}

DenseMatrix::size_type DenseMatrix::size() const noexcept
{
    return values_.size();
}

double &DenseMatrix::operator()(const size_type row, const size_type column)
{
    return values_[index(row, column)];
}

const double &DenseMatrix::operator()(const size_type row, const size_type column) const
{
    return values_[index(row, column)];
}

DenseMatrix::size_type DenseMatrix::index(const size_type row, const size_type column) const
{
    if (row >= rows_ || column >= columns_)
    {
        throw std::out_of_range("DenseMatrix index is out of range.");
    }

    return row * columns_ + column;
}
} // namespace finelemethod::math
