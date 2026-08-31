#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace finelemethod::math
{
struct CooEntry
{
    std::size_t row;
    std::size_t column;
    double value;
};

// Sparse matrix in coordinate form. Duplicate coordinates are intentionally
// retained so element contributions can be appended without searching.
class CooMatrix
{
  public:
    using size_type = std::size_t;

    CooMatrix(size_type rows, size_type columns);

    [[nodiscard]] size_type rows() const noexcept;
    [[nodiscard]] size_type columns() const noexcept;
    [[nodiscard]] size_type nonzero_count() const noexcept;
    [[nodiscard]] std::span<const CooEntry> entries() const noexcept;

    // Appends a finite nonzero contribution. Zero values are ignored.
    void add(size_type row, size_type column, double value);

  private:
    size_type rows_;
    size_type columns_;
    std::vector<CooEntry> entries_;
};
} // namespace finelemethod::math
