#include "finelemethod/math/dense_matrix.hpp"

#include "finelemethod/math/dense_vector.hpp"

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

DenseMatrix &DenseMatrix::operator+=(const DenseMatrix &other)
{
    if (rows_ != other.rows_ || columns_ != other.columns_)
    {
        throw std::invalid_argument("DenseMatrix dimensions must match for addition.");
    }

    for (size_type index_value = 0; index_value < values_.size(); ++index_value)
    {
        values_[index_value] += other.values_[index_value];
    }

    return *this;
}

DenseMatrix &DenseMatrix::operator-=(const DenseMatrix &other)
{
    if (rows_ != other.rows_ || columns_ != other.columns_)
    {
        throw std::invalid_argument("DenseMatrix dimensions must match for subtraction.");
    }

    for (size_type index_value = 0; index_value < values_.size(); ++index_value)
    {
        values_[index_value] -= other.values_[index_value];
    }

    return *this;
}

DenseMatrix &DenseMatrix::operator*=(const double scalar) noexcept
{
    for (double &value : values_)
    {
        value *= scalar;
    }

    return *this;
}

DenseMatrix::size_type DenseMatrix::index(const size_type row, const size_type column) const
{
    if (row >= rows_ || column >= columns_)
    {
        throw std::out_of_range("DenseMatrix index is out of range.");
    }

    return row * columns_ + column;
}

DenseMatrix operator+(DenseMatrix left, const DenseMatrix &right)
{
    left += right;
    return left;
}

DenseMatrix operator-(DenseMatrix left, const DenseMatrix &right)
{
    left -= right;
    return left;
}

DenseMatrix operator*(DenseMatrix matrix, const double scalar) noexcept
{
    matrix *= scalar;
    return matrix;
}

DenseMatrix operator*(const double scalar, DenseMatrix matrix) noexcept
{
    matrix *= scalar;
    return matrix;
}

DenseMatrix operator*(const DenseMatrix &left, const DenseMatrix &right)
{
    if (left.columns() != right.rows())
    {
        throw std::invalid_argument("DenseMatrix inner dimensions must match for multiplication.");
    }

    DenseMatrix result(left.rows(), right.columns());

    for (DenseMatrix::size_type row = 0; row < left.rows(); ++row)
    {
        for (DenseMatrix::size_type inner = 0; inner < left.columns(); ++inner)
        {
            const double left_value = left(row, inner);
            for (DenseMatrix::size_type column = 0; column < right.columns(); ++column)
            {
                result(row, column) += left_value * right(inner, column);
            }
        }
    }

    return result;
}

DenseVector operator*(const DenseMatrix &matrix, const DenseVector &vector)
{
    if (matrix.columns() != vector.size())
    {
        throw std::invalid_argument(
            "DenseMatrix columns must match DenseVector size for multiplication.");
    }

    DenseVector result(matrix.rows());

    for (DenseMatrix::size_type row = 0; row < matrix.rows(); ++row)
    {
        for (DenseMatrix::size_type column = 0; column < matrix.columns(); ++column)
        {
            result[row] += matrix(row, column) * vector[column];
        }
    }

    return result;
}

DenseMatrix transpose(const DenseMatrix &matrix)
{
    DenseMatrix result(matrix.columns(), matrix.rows());

    for (DenseMatrix::size_type row = 0; row < matrix.rows(); ++row)
    {
        for (DenseMatrix::size_type column = 0; column < matrix.columns(); ++column)
        {
            result(column, row) = matrix(row, column);
        }
    }

    return result;
}
} // namespace finelemethod::math
