#pragma once

#include <filesystem>

namespace finelemethod::examples
{
// Solves a unit H8 block under uniform uniaxial compression and writes all
// results to VTU.
void write_h8_compression_example(const std::filesystem::path &output_path);
} // namespace finelemethod::examples
