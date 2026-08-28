#pragma once

#include "finelemethod/input/abaqus_parse_error.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace finelemethod::input
{
struct AbaqusIsotropicMaterial
{
    std::string name;
    double youngs_modulus;
    double poisson_ratio;
};

// Parses named *MATERIAL definitions with isotropic *ELASTIC data.
[[nodiscard]] std::vector<AbaqusIsotropicMaterial> parse_abaqus_isotropic_materials(
    std::string_view input_text);
} // namespace finelemethod::input
