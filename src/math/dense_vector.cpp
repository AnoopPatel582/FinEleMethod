#include "finelemethod/math/dense_vector.hpp"

#include <stdexcept>

namespace finelemethod::math
{
namespace
{
DenseVector::size_type checked_size(const DenseVector::size_type size)
{
    if (size == 0)
    {
        throw std::invalid_argument("DenseVector size must be greater than zero.");
    }

    return size;
}
} // namespace

DenseVector::DenseVector(const size_type size, const double initial_value)
    : values_(checked_size(size), initial_value)
{
}

DenseVector::size_type DenseVector::size() const noexcept
{
    return values_.size();
}

double &DenseVector::operator[](const size_type index)
{
    return values_.at(index);
}

const double &DenseVector::operator[](const size_type index) const
{
    return values_.at(index);
}
} // namespace finelemethod::math
