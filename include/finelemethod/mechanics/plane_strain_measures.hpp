#pragma once

#include <array>

namespace finelemethod::mechanics
{
struct PlaneStrainMeasures
{
    double von_mises;
    // Descending order [sigma_1, sigma_2, sigma_3].
    std::array<double, 3> principal_stresses;
};

[[nodiscard]] PlaneStrainMeasures calculate_plane_strain_measures(double sigma_x, double sigma_y,
                                                                  double sigma_z, double tau_xy);
} // namespace finelemethod::mechanics
