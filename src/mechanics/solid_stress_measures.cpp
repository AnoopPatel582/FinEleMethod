#include "finelemethod/mechanics/solid_stress_measures.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <stdexcept>

namespace finelemethod::mechanics
{
SolidStressMeasures calculate_solid_stress_measures(const double sigma_x, const double sigma_y,
                                                    const double sigma_z, const double tau_xy,
                                                    const double tau_yz, const double tau_zx)
{
    if (!std::isfinite(sigma_x) || !std::isfinite(sigma_y) || !std::isfinite(sigma_z) ||
        !std::isfinite(tau_xy) || !std::isfinite(tau_yz) || !std::isfinite(tau_zx))
    {
        throw std::invalid_argument("Solid stress components must be finite.");
    }

    const double von_mises = std::sqrt(0.5 * ((sigma_x - sigma_y) * (sigma_x - sigma_y) +
                                              (sigma_y - sigma_z) * (sigma_y - sigma_z) +
                                              (sigma_z - sigma_x) * (sigma_z - sigma_x)) +
                                       3.0 * (tau_xy * tau_xy + tau_yz * tau_yz + tau_zx * tau_zx));

    std::array<std::array<double, 3>, 3> stress_tensor{{
        {{sigma_x, tau_xy, tau_zx}},
        {{tau_xy, sigma_y, tau_yz}},
        {{tau_zx, tau_yz, sigma_z}},
    }};
    const double stress_scale = std::max({std::abs(sigma_x), std::abs(sigma_y), std::abs(sigma_z),
                                          std::abs(tau_xy), std::abs(tau_yz), std::abs(tau_zx)});
    const double tolerance = 64.0 * std::numeric_limits<double>::epsilon() * stress_scale;

    for (std::size_t iteration = 0; iteration < 20; ++iteration)
    {
        std::size_t first = 0;
        std::size_t second = 1;
        double largest = std::abs(stress_tensor[first][second]);
        for (const auto pair : {std::array<std::size_t, 2>{0, 2}, std::array<std::size_t, 2>{1, 2}})
        {
            const double candidate = std::abs(stress_tensor[pair[0]][pair[1]]);
            if (candidate > largest)
            {
                first = pair[0];
                second = pair[1];
                largest = candidate;
            }
        }
        if (largest <= tolerance)
        {
            break;
        }

        const double angle =
            0.5 * std::atan2(2.0 * stress_tensor[first][second],
                             stress_tensor[second][second] - stress_tensor[first][first]);
        const double cosine = std::cos(angle);
        const double sine = std::sin(angle);
        const double first_diagonal = stress_tensor[first][first];
        const double second_diagonal = stress_tensor[second][second];
        const double coupling = stress_tensor[first][second];
        stress_tensor[first][first] = cosine * cosine * first_diagonal -
                                      2.0 * sine * cosine * coupling +
                                      sine * sine * second_diagonal;
        stress_tensor[second][second] = sine * sine * first_diagonal +
                                        2.0 * sine * cosine * coupling +
                                        cosine * cosine * second_diagonal;
        stress_tensor[first][second] = 0.0;
        stress_tensor[second][first] = 0.0;

        for (std::size_t index = 0; index < 3; ++index)
        {
            if (index == first || index == second)
            {
                continue;
            }
            const double first_value = stress_tensor[index][first];
            const double second_value = stress_tensor[index][second];
            stress_tensor[index][first] = cosine * first_value - sine * second_value;
            stress_tensor[first][index] = stress_tensor[index][first];
            stress_tensor[index][second] = sine * first_value + cosine * second_value;
            stress_tensor[second][index] = stress_tensor[index][second];
        }
    }

    std::array<double, 3> principal_stresses{stress_tensor[0][0], stress_tensor[1][1],
                                             stress_tensor[2][2]};
    std::sort(principal_stresses.begin(), principal_stresses.end(), std::greater<>{});

    return {von_mises, principal_stresses};
}
} // namespace finelemethod::mechanics
