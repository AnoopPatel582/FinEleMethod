#include "finelemethod/mechanics/plane_stress_measures.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>

namespace finelemethod::mechanics
{
PlaneStressMeasures calculate_plane_stress_measures(const double sigma_x, const double sigma_y,
                                                    const double tau_xy)
{
    if (!std::isfinite(sigma_x) || !std::isfinite(sigma_y) || !std::isfinite(tau_xy))
    {
        throw std::invalid_argument("Plane-stress components must be finite.");
    }

    const double mean_stress = 0.5 * (sigma_x + sigma_y);
    const double half_difference = 0.5 * (sigma_x - sigma_y);
    const double radius = std::hypot(half_difference, tau_xy);

    std::array principal_stresses{mean_stress + radius, mean_stress - radius, 0.0};
    std::ranges::sort(principal_stresses, std::greater<>{});

    const double von_mises = std::sqrt(sigma_x * sigma_x - sigma_x * sigma_y + sigma_y * sigma_y +
                                       3.0 * tau_xy * tau_xy);

    return PlaneStressMeasures{von_mises, principal_stresses};
}
} // namespace finelemethod::mechanics
