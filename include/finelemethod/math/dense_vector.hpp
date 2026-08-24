#pragma once

#include <cstddef>
#include <vector>

namespace finelemethod::math
{
class DenseVector
{
  public:
    using size_type = std::size_t;

    explicit DenseVector(size_type size, double initial_value = 0.0);

    [[nodiscard]] size_type size() const noexcept;

    double &operator[](size_type index);
    const double &operator[](size_type index) const;

  private:
    std::vector<double> values_;
};
} // namespace finelemethod::math
