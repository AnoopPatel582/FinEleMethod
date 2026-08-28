#pragma once

#include <filesystem>

namespace finelemethod::examples
{
// Solves a unit Q4 under uniform uniaxial tension and writes all results to VTU.
void write_q4_tension_example(const std::filesystem::path &output_path);
} // namespace finelemethod::examples
