#pragma once

#include <array>

namespace finelemethod::mechanics
{
struct SolidStressMeasures
{
    double von_mises;
    // Descending order [sigma_1, sigma_2, sigma_3].
    std::array<double, 3> principal_stresses;
};

// Stress order: [sigma_x, sigma_y, sigma_z, tau_xy, tau_yz, tau_zx].
[[nodiscard]] SolidStressMeasures calculate_solid_stress_measures(double sigma_x, double sigma_y,
                                                                  double sigma_z, double tau_xy,
                                                                  double tau_yz, double tau_zx);
} // namespace finelemethod::mechanics
