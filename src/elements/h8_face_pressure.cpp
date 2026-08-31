#include "finelemethod/elements/h8_face_pressure.hpp"

#include <array>
#include <cmath>
#include <stdexcept>

namespace finelemethod::elements
{
namespace
{
using Vector3 = std::array<double, 3>;

Vector3 cross(const Vector3 &left, const Vector3 &right)
{
    return Vector3{{left[1] * right[2] - left[2] * right[1],
                    left[2] * right[0] - left[0] * right[2],
                    left[0] * right[1] - left[1] * right[0]}};
}

double dot(const Vector3 &left, const Vector3 &right)
{
    return left[0] * right[0] + left[1] * right[1] + left[2] * right[2];
}
} // namespace

math::DenseVector h8_uniform_face_pressure_load(const H8NodeCoordinates &coordinates,
                                                const model::H8Face face, const double pressure)
{
    if (!std::isfinite(pressure))
    {
        throw std::invalid_argument("H8 face pressure must be finite.");
    }
    for (const auto &coordinate : coordinates)
    {
        for (const double component : coordinate)
        {
            if (!std::isfinite(component))
            {
                throw std::invalid_argument("H8 face pressure coordinates must be finite.");
            }
        }
    }

    // Cyclic node order for ABAQUS faces S1 through S6.
    constexpr std::array<std::array<std::size_t, 4>, 6> face_nodes{{
        {{0, 3, 2, 1}},
        {{4, 5, 6, 7}},
        {{0, 1, 5, 4}},
        {{1, 2, 6, 5}},
        {{2, 3, 7, 6}},
        {{3, 0, 4, 7}},
    }};
    const std::size_t face_index = static_cast<std::size_t>(face);
    if (face_index >= face_nodes.size())
    {
        throw std::invalid_argument("H8 face number must be between one and six.");
    }

    Vector3 element_centroid{};
    for (const auto &coordinate : coordinates)
    {
        for (std::size_t component = 0; component < 3; ++component)
        {
            element_centroid[component] += coordinate[component] / 8.0;
        }
    }

    constexpr double gauss_coordinate = 0.57735026918962576451;
    constexpr std::array<double, 2> gauss_points{{-gauss_coordinate, gauss_coordinate}};
    math::DenseVector equivalent_load(24);

    for (const double s : gauss_points)
    {
        for (const double t : gauss_points)
        {
            const std::array<double, 4> shape{{
                0.25 * (1.0 - s) * (1.0 - t),
                0.25 * (1.0 + s) * (1.0 - t),
                0.25 * (1.0 + s) * (1.0 + t),
                0.25 * (1.0 - s) * (1.0 + t),
            }};
            const std::array<double, 4> derivative_s{{
                -0.25 * (1.0 - t),
                0.25 * (1.0 - t),
                0.25 * (1.0 + t),
                -0.25 * (1.0 + t),
            }};
            const std::array<double, 4> derivative_t{{
                -0.25 * (1.0 - s),
                -0.25 * (1.0 + s),
                0.25 * (1.0 + s),
                0.25 * (1.0 - s),
            }};

            Vector3 point{};
            Vector3 tangent_s{};
            Vector3 tangent_t{};
            for (std::size_t local_node = 0; local_node < 4; ++local_node)
            {
                const auto &coordinate = coordinates[face_nodes[face_index][local_node]];
                for (std::size_t component = 0; component < 3; ++component)
                {
                    point[component] += shape[local_node] * coordinate[component];
                    tangent_s[component] += derivative_s[local_node] * coordinate[component];
                    tangent_t[component] += derivative_t[local_node] * coordinate[component];
                }
            }

            Vector3 area_vector = cross(tangent_s, tangent_t);
            const double area_scale = std::sqrt(dot(area_vector, area_vector));
            if (!std::isfinite(area_scale) || area_scale <= 0.0)
            {
                throw std::invalid_argument("H8 pressure face must have positive area.");
            }
            Vector3 toward_centroid{};
            for (std::size_t component = 0; component < 3; ++component)
            {
                toward_centroid[component] = element_centroid[component] - point[component];
            }
            if (dot(area_vector, toward_centroid) < 0.0)
            {
                for (double &component : area_vector)
                {
                    component = -component;
                }
            }

            for (std::size_t local_node = 0; local_node < 4; ++local_node)
            {
                const std::size_t element_node = face_nodes[face_index][local_node];
                for (std::size_t component = 0; component < 3; ++component)
                {
                    equivalent_load[3 * element_node + component] +=
                        pressure * shape[local_node] * area_vector[component];
                }
            }
        }
    }
    return equivalent_load;
}
} // namespace finelemethod::elements
