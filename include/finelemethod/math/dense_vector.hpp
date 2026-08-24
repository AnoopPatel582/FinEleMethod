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

    DenseVector &operator+=(const DenseVector &other);
    DenseVector &operator-=(const DenseVector &other);
    DenseVector &operator*=(double scalar) noexcept;

  private:
    std::vector<double> values_;
};

[[nodiscard]] DenseVector operator+(DenseVector left, const DenseVector &right);
[[nodiscard]] DenseVector operator-(DenseVector left, const DenseVector &right);
[[nodiscard]] DenseVector operator*(DenseVector vector, double scalar) noexcept;
[[nodiscard]] DenseVector operator*(double scalar, DenseVector vector) noexcept;
[[nodiscard]] double dot(const DenseVector &left, const DenseVector &right);
[[nodiscard]] double norm(const DenseVector &vector);
} // namespace finelemethod::math
