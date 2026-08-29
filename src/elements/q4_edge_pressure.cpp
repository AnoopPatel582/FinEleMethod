#include "finelemethod/elements/q4_edge_pressure.hpp"

#include <array>
#include <cmath>
#include <stdexcept>

namespace finelemethod::elements
{
math::DenseVector q4_uniform_edge_pressure_load(const Q4NodeCoordinates &coordinates,
                                                const model::Q4Edge edge, const double pressure,
                                                const double thickness)
{
    if (!std::isfinite(pressure))
    {
        throw std::invalid_argument("Q4 edge pressure must be finite.");
    }
    if (!std::isfinite(thickness) || thickness <= 0.0)
    {
        throw std::invalid_argument("Q4 edge pressure thickness must be positive and finite.");
    }
    for (const auto &coordinate : coordinates)
    {
        if (!std::isfinite(coordinate[0]) || !std::isfinite(coordinate[1]))
        {
            throw std::invalid_argument("Q4 edge pressure coordinates must be finite.");
        }
    }

    double signed_area_twice = 0.0;
    for (std::size_t node = 0; node < coordinates.size(); ++node)
    {
        const std::size_t next = (node + 1) % coordinates.size();
        signed_area_twice += coordinates[node][0] * coordinates[next][1] -
                             coordinates[next][0] * coordinates[node][1];
    }
    if (signed_area_twice <= 0.0)
    {
        throw std::invalid_argument(
            "Q4 edge pressure requires counterclockwise nondegenerate coordinates.");
    }

    constexpr std::array<std::array<std::size_t, 2>, 4> edge_nodes{
        {{{0, 1}}, {{1, 2}}, {{2, 3}}, {{3, 0}}}};
    const std::size_t edge_index = static_cast<std::size_t>(edge);
    if (edge_index >= edge_nodes.size())
    {
        throw std::invalid_argument("Q4 edge number must be between one and four.");
    }

    const std::size_t first_node = edge_nodes[edge_index][0];
    const std::size_t second_node = edge_nodes[edge_index][1];
    const double delta_x = coordinates[second_node][0] - coordinates[first_node][0];
    const double delta_y = coordinates[second_node][1] - coordinates[first_node][1];
    if (delta_x == 0.0 && delta_y == 0.0)
    {
        throw std::invalid_argument("Q4 pressure edge must have positive length.");
    }

    const double nodal_force_x = -pressure * thickness * delta_y / 2.0;
    const double nodal_force_y = pressure * thickness * delta_x / 2.0;
    math::DenseVector equivalent_load(8);
    equivalent_load[2 * first_node] = nodal_force_x;
    equivalent_load[2 * first_node + 1] = nodal_force_y;
    equivalent_load[2 * second_node] = nodal_force_x;
    equivalent_load[2 * second_node + 1] = nodal_force_y;
    return equivalent_load;
}
} // namespace finelemethod::elements
