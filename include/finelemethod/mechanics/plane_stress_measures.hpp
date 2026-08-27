#pragma once

#include <array>

namespace finelemethod::mechanics
{
struct PlaneStressMeasures
{
    double von_mises;
    // Descending order [sigma_1, sigma_2, sigma_3], including the
    // out-of-plane plane-stress value sigma_z = 0.
    std::array<double, 3> principal_stresses;
};

[[nodiscard]] PlaneStressMeasures calculate_plane_stress_measures(double sigma_x, double sigma_y,
                                                                  double tau_xy);
} // namespace finelemethod::mechanics
