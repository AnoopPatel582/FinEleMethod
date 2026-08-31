#include "finelemethod/math/csr_matrix.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace finelemethod::math
{
CsrMatrix::CsrMatrix(const size_type rows, const size_type columns,
                     std::vector<size_type> row_offsets, std::vector<size_type> column_indices,
                     std::vector<double> values)
    : rows_(rows), columns_(columns), row_offsets_(std::move(row_offsets)),
      column_indices_(std::move(column_indices)), values_(std::move(values))
{
}

CsrMatrix::size_type CsrMatrix::rows() const noexcept
{
    return rows_;
}

CsrMatrix::size_type CsrMatrix::columns() const noexcept
{
    return columns_;
}

CsrMatrix::size_type CsrMatrix::nonzero_count() const noexcept
{
    return values_.size();
}

std::span<const CsrMatrix::size_type> CsrMatrix::row_offsets() const noexcept
{
    return row_offsets_;
}

std::span<const CsrMatrix::size_type> CsrMatrix::column_indices() const noexcept
{
    return column_indices_;
}

std::span<const double> CsrMatrix::values() const noexcept
{
    return values_;
}

CsrMatrix convert_to_csr(const CooMatrix &matrix)
{
    std::vector<CooEntry> sorted_entries(matrix.entries().begin(), matrix.entries().end());
    std::sort(sorted_entries.begin(), sorted_entries.end(),
              [](const CooEntry &left, const CooEntry &right) {
                  if (left.row != right.row)
                  {
                      return left.row < right.row;
                  }
                  return left.column < right.column;
              });

    std::vector<CsrMatrix::size_type> row_offsets;
    std::vector<CsrMatrix::size_type> column_indices;
    std::vector<double> values;
    row_offsets.reserve(matrix.rows() + 1);
    column_indices.reserve(sorted_entries.size());
    values.reserve(sorted_entries.size());
    row_offsets.push_back(0);

    std::size_t entry_index = 0;
    for (std::size_t row = 0; row < matrix.rows(); ++row)
    {
        while (entry_index < sorted_entries.size() && sorted_entries[entry_index].row == row)
        {
            const std::size_t column = sorted_entries[entry_index].column;
            double assembled_value = 0.0;
            while (entry_index < sorted_entries.size() && sorted_entries[entry_index].row == row &&
                   sorted_entries[entry_index].column == column)
            {
                assembled_value += sorted_entries[entry_index].value;
                if (!std::isfinite(assembled_value))
                {
                    throw std::overflow_error("Assembled CSR value is not finite.");
                }
                ++entry_index;
            }

            if (assembled_value != 0.0)
            {
                column_indices.push_back(column);
                values.push_back(assembled_value);
            }
        }
        row_offsets.push_back(values.size());
    }

    return CsrMatrix(matrix.rows(), matrix.columns(), std::move(row_offsets),
                     std::move(column_indices), std::move(values));
}
} // namespace finelemethod::math
