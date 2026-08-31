#include "finelemethod/math/coo_matrix.hpp"

#include <cmath>
#include <stdexcept>

namespace finelemethod::math
{
CooMatrix::CooMatrix(const size_type rows, const size_type columns) : rows_(rows), columns_(columns)
{
    if (rows == 0 || columns == 0)
    {
        throw std::invalid_argument("CooMatrix dimensions must be greater than zero.");
    }
}

CooMatrix::size_type CooMatrix::rows() const noexcept
{
    return rows_;
}

CooMatrix::size_type CooMatrix::columns() const noexcept
{
    return columns_;
}

CooMatrix::size_type CooMatrix::nonzero_count() const noexcept
{
    return entries_.size();
}

std::span<const CooEntry> CooMatrix::entries() const noexcept
{
    return entries_;
}

void CooMatrix::add(const size_type row, const size_type column, const double value)
{
    if (row >= rows_ || column >= columns_)
    {
        throw std::out_of_range("CooMatrix index is out of range.");
    }
    if (!std::isfinite(value))
    {
        throw std::invalid_argument("CooMatrix values must be finite.");
    }
    if (value == 0.0)
    {
        return;
    }

    entries_.push_back(CooEntry{row, column, value});
}
} // namespace finelemethod::math
