#pragma once

#include <cstddef>
#include <vector>

namespace finelemethod::math
{
class DenseMatrix
{
  public:
    using size_type = std::size_t;

    DenseMatrix(size_type rows, size_type columns, double initial_value = 0.0);

    [[nodiscard]] size_type rows() const noexcept;
    [[nodiscard]] size_type columns() const noexcept;
    [[nodiscard]] size_type size() const noexcept;

    double &operator()(size_type row, size_type column);
    const double &operator()(size_type row, size_type column) const;

    DenseMatrix &operator+=(const DenseMatrix &other);

  private:
    [[nodiscard]] size_type index(size_type row, size_type column) const;

    size_type rows_;
    size_type columns_;
    std::vector<double> values_;
};

[[nodiscard]] DenseMatrix operator+(DenseMatrix left, const DenseMatrix &right);
} // namespace finelemethod::math
