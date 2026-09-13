#include "finelemethod/elements/t4.hpp"

#include "finelemethod/mechanics/constitutive_matrix.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

namespace finelemethod::elements
{
namespace
{
using Vector3 = std::array<double, 3>;

Vector3 subtract(const Vector3 &left, const Vector3 &right)
{
    return {{left[0] - right[0], left[1] - right[1], left[2] - right[2]}};
}

Vector3 cross(const Vector3 &left, const Vector3 &right)
{
    return {{left[1] * right[2] - left[2] * right[1], left[2] * right[0] - left[0] * right[2],
             left[0] * right[1] - left[1] * right[0]}};
}

double dot(const Vector3 &left, const Vector3 &right)
{
    return left[0] * right[0] + left[1] * right[1] + left[2] * right[2];
}

double jacobian_determinant(const T4NodeCoordinates &coordinates)
{
    return dot(
        subtract(coordinates[1], coordinates[0]),
        cross(subtract(coordinates[2], coordinates[0]), subtract(coordinates[3], coordinates[0])));
}

void validate_coordinates(const T4NodeCoordinates &coordinates)
{
    double scale = 0.0;
    for (const auto &coordinate : coordinates)
    {
        for (const double component : coordinate)
        {
            if (!std::isfinite(component))
            {
                throw std::invalid_argument("T4 nodal coordinates must be finite.");
            }
            scale = std::max(scale, std::abs(component));
        }
    }
    const double determinant = jacobian_determinant(coordinates);
    const double tolerance =
        64.0 * std::numeric_limits<double>::epsilon() * std::max(1.0, scale * scale * scale);
    if (!std::isfinite(determinant) || determinant <= tolerance)
    {
        throw std::invalid_argument("T4 volume must be positive and non-degenerate.");
    }
}
} // namespace

T4StrainDisplacement t4_strain_displacement_matrix(const T4NodeCoordinates &coordinates)
{
    validate_coordinates(coordinates);
    math::DenseMatrix jacobian(3, 3);
    for (std::size_t column = 0; column < 3; ++column)
    {
        jacobian(0, column) = coordinates[1][column] - coordinates[0][column];
        jacobian(1, column) = coordinates[2][column] - coordinates[0][column];
        jacobian(2, column) = coordinates[3][column] - coordinates[0][column];
    }
    const double a = jacobian(0, 0);
    const double b = jacobian(0, 1);
    const double c = jacobian(0, 2);
    const double d = jacobian(1, 0);
    const double e = jacobian(1, 1);
    const double f = jacobian(1, 2);
    const double g = jacobian(2, 0);
    const double h = jacobian(2, 1);
    const double i = jacobian(2, 2);
    const double determinant = jacobian_determinant(coordinates);

    math::DenseMatrix inverse(3, 3);
    inverse(0, 0) = (e * i - f * h) / determinant;
    inverse(0, 1) = (c * h - b * i) / determinant;
    inverse(0, 2) = (b * f - c * e) / determinant;
    inverse(1, 0) = (f * g - d * i) / determinant;
    inverse(1, 1) = (a * i - c * g) / determinant;
    inverse(1, 2) = (c * d - a * f) / determinant;
    inverse(2, 0) = (d * h - e * g) / determinant;
    inverse(2, 1) = (b * g - a * h) / determinant;
    inverse(2, 2) = (a * e - b * d) / determinant;

    constexpr std::array<std::array<double, 3>, 4> natural_derivatives{{
        {{-1.0, -1.0, -1.0}},
        {{1.0, 0.0, 0.0}},
        {{0.0, 1.0, 0.0}},
        {{0.0, 0.0, 1.0}},
    }};
    math::DenseMatrix matrix(6, 12);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        std::array<double, 3> derivative{};
        for (std::size_t physical = 0; physical < 3; ++physical)
        {
            for (std::size_t natural = 0; natural < 3; ++natural)
            {
                derivative[physical] +=
                    inverse(physical, natural) * natural_derivatives[node][natural];
            }
        }
        const std::size_t x = 3 * node;
        const std::size_t y = x + 1;
        const std::size_t z = x + 2;
        matrix(0, x) = derivative[0];
        matrix(1, y) = derivative[1];
        matrix(2, z) = derivative[2];
        matrix(3, x) = derivative[1];
        matrix(3, y) = derivative[0];
        matrix(4, y) = derivative[2];
        matrix(4, z) = derivative[1];
        matrix(5, x) = derivative[2];
        matrix(5, z) = derivative[0];
    }
    return {std::move(matrix), determinant / 6.0};
}

math::DenseMatrix t4_stiffness_matrix(const T4NodeCoordinates &coordinates,
                                      const model::IsotropicElasticMaterial &material)
{
    const T4StrainDisplacement strain_displacement = t4_strain_displacement_matrix(coordinates);
    const math::DenseMatrix constitutive = mechanics::solid_isotropic_constitutive_matrix(material);
    return transpose(strain_displacement.matrix) * constitutive * strain_displacement.matrix *
           strain_displacement.volume;
}

math::DenseVector t4_uniform_face_pressure_load(const T4NodeCoordinates &coordinates,
                                                const T4Face face, const double pressure)
{
    validate_coordinates(coordinates);
    if (!std::isfinite(pressure))
    {
        throw std::invalid_argument("T4 face pressure must be finite.");
    }
    // ABAQUS C3D4 face numbering with cyclic outward orientation.
    constexpr std::array<std::array<std::size_t, 3>, 4> face_nodes{{
        {{0, 2, 1}},
        {{0, 1, 3}},
        {{1, 2, 3}},
        {{2, 0, 3}},
    }};
    const std::size_t face_index = static_cast<std::size_t>(face);
    if (face_index >= face_nodes.size())
    {
        throw std::invalid_argument("T4 face number must be between one and four.");
    }
    const auto &nodes = face_nodes[face_index];
    Vector3 area_vector = cross(subtract(coordinates[nodes[1]], coordinates[nodes[0]]),
                                subtract(coordinates[nodes[2]], coordinates[nodes[0]]));
    // cross product magnitude is twice the triangle area. Positive pressure is inward.
    math::DenseVector load(12);
    for (const std::size_t node : nodes)
    {
        for (std::size_t component = 0; component < 3; ++component)
        {
            load[3 * node + component] = -pressure * area_vector[component] / 6.0;
        }
    }
    return load;
}
} // namespace finelemethod::elements
