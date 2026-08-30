#include "finelemethod/mechanics/plane_strain_measures.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>

namespace finelemethod::mechanics
{
PlaneStrainMeasures calculate_plane_strain_measures(const double sigma_x, const double sigma_y,
                                                    const double sigma_z, const double tau_xy)
{
    if (!std::isfinite(sigma_x) || !std::isfinite(sigma_y) || !std::isfinite(sigma_z) ||
        !std::isfinite(tau_xy))
    {
        throw std::invalid_argument("Plane-strain stress components must be finite.");
    }

    const double mean_stress = 0.5 * (sigma_x + sigma_y);
    const double half_difference = 0.5 * (sigma_x - sigma_y);
    const double radius = std::hypot(half_difference, tau_xy);
    std::array principal_stresses{mean_stress + radius, mean_stress - radius, sigma_z};
    std::ranges::sort(principal_stresses, std::greater<>{});

    const double von_mises = std::sqrt(0.5 * ((sigma_x - sigma_y) * (sigma_x - sigma_y) +
                                              (sigma_y - sigma_z) * (sigma_y - sigma_z) +
                                              (sigma_z - sigma_x) * (sigma_z - sigma_x)) +
                                       3.0 * tau_xy * tau_xy);
    return PlaneStrainMeasures{von_mises, principal_stresses};
}
} // namespace finelemethod::mechanics
