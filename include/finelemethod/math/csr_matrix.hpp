#pragma once

#include "finelemethod/math/coo_matrix.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace finelemethod::math
{
class DenseVector;

class CsrMatrix
{
  public:
    using size_type = std::size_t;

    [[nodiscard]] size_type rows() const noexcept;
    [[nodiscard]] size_type columns() const noexcept;
    [[nodiscard]] size_type nonzero_count() const noexcept;
    [[nodiscard]] std::span<const size_type> row_offsets() const noexcept;
    [[nodiscard]] std::span<const size_type> column_indices() const noexcept;
    [[nodiscard]] std::span<const double> values() const noexcept;

  private:
    friend CsrMatrix convert_to_csr(const CooMatrix &matrix);

    CsrMatrix(size_type rows, size_type columns, std::vector<size_type> row_offsets,
              std::vector<size_type> column_indices, std::vector<double> values);

    size_type rows_;
    size_type columns_;
    std::vector<size_type> row_offsets_;
    std::vector<size_type> column_indices_;
    std::vector<double> values_;
};

// Sorts coordinate entries into row-major order, sums duplicate coordinates,
// and removes entries whose assembled value is exactly zero.
[[nodiscard]] CsrMatrix convert_to_csr(const CooMatrix &matrix);
[[nodiscard]] DenseVector operator*(const CsrMatrix &matrix, const DenseVector &vector);
} // namespace finelemethod::math
