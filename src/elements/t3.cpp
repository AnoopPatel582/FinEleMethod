#include "finelemethod/elements/t3.hpp"

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
double signed_area_twice(const T3NodeCoordinates &coordinates)
{
    return (coordinates[1][0] - coordinates[0][0]) * (coordinates[2][1] - coordinates[0][1]) -
           (coordinates[2][0] - coordinates[0][0]) * (coordinates[1][1] - coordinates[0][1]);
}

void validate_coordinates(const T3NodeCoordinates &coordinates)
{
    double scale = 0.0;
    for (const auto &coordinate : coordinates)
    {
        if (!std::isfinite(coordinate[0]) || !std::isfinite(coordinate[1]))
        {
            throw std::invalid_argument("T3 nodal coordinates must be finite.");
        }
        scale = std::max({scale, std::abs(coordinate[0]), std::abs(coordinate[1])});
    }
    const double area_twice = signed_area_twice(coordinates);
    const double tolerance =
        16.0 * std::numeric_limits<double>::epsilon() * std::max(1.0, scale * scale);
    if (!std::isfinite(area_twice) || area_twice <= tolerance)
    {
        throw std::invalid_argument("T3 area must be positive and non-degenerate.");
    }
}
} // namespace

T3StrainDisplacement t3_strain_displacement_matrix(const T3NodeCoordinates &coordinates)
{
    validate_coordinates(coordinates);
    const double area_twice = signed_area_twice(coordinates);
    const std::array<double, 3> derivative_x{{
        (coordinates[1][1] - coordinates[2][1]) / area_twice,
        (coordinates[2][1] - coordinates[0][1]) / area_twice,
        (coordinates[0][1] - coordinates[1][1]) / area_twice,
    }};
    const std::array<double, 3> derivative_y{{
        (coordinates[2][0] - coordinates[1][0]) / area_twice,
        (coordinates[0][0] - coordinates[2][0]) / area_twice,
        (coordinates[1][0] - coordinates[0][0]) / area_twice,
    }};

    math::DenseMatrix matrix(3, 6);
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        matrix(0, 2 * node) = derivative_x[node];
        matrix(1, 2 * node + 1) = derivative_y[node];
        matrix(2, 2 * node) = derivative_y[node];
        matrix(2, 2 * node + 1) = derivative_x[node];
    }
    return {std::move(matrix), 0.5 * area_twice};
}

math::DenseMatrix t3_plane_stress_stiffness_matrix(const T3NodeCoordinates &coordinates,
                                                   const model::IsotropicElasticMaterial &material,
                                                   const double thickness)
{
    if (!std::isfinite(thickness) || thickness <= 0.0)
    {
        throw std::invalid_argument("T3 thickness must be positive and finite.");
    }
    const T3StrainDisplacement strain_displacement = t3_strain_displacement_matrix(coordinates);
    const math::DenseMatrix constitutive = mechanics::plane_stress_constitutive_matrix(material);
    return transpose(strain_displacement.matrix) * constitutive * strain_displacement.matrix *
           (strain_displacement.area * thickness);
}

math::DenseVector t3_uniform_edge_pressure_load(const T3NodeCoordinates &coordinates,
                                                const T3Edge edge, const double pressure,
                                                const double thickness)
{
    validate_coordinates(coordinates);
    if (!std::isfinite(pressure))
    {
        throw std::invalid_argument("T3 edge pressure must be finite.");
    }
    if (!std::isfinite(thickness) || thickness <= 0.0)
    {
        throw std::invalid_argument("T3 pressure thickness must be positive and finite.");
    }
    constexpr std::array<std::array<std::size_t, 2>, 3> edge_nodes{{
        {{0, 1}},
        {{1, 2}},
        {{2, 0}},
    }};
    const std::size_t edge_index = static_cast<std::size_t>(edge);
    if (edge_index >= edge_nodes.size())
    {
        throw std::invalid_argument("T3 edge number must be between one and three.");
    }
    const std::size_t first = edge_nodes[edge_index][0];
    const std::size_t second = edge_nodes[edge_index][1];
    const double delta_x = coordinates[second][0] - coordinates[first][0];
    const double delta_y = coordinates[second][1] - coordinates[first][1];

    math::DenseVector load(6);
    const double force_x = -pressure * thickness * delta_y / 2.0;
    const double force_y = pressure * thickness * delta_x / 2.0;
    load[2 * first] = force_x;
    load[2 * first + 1] = force_y;
    load[2 * second] = force_x;
    load[2 * second + 1] = force_y;
    return load;
}
} // namespace finelemethod::elements
