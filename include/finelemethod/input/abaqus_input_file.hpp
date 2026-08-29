#pragma once

#include <filesystem>
#include <string>

namespace finelemethod::input
{
// Reads the complete contents of an ABAQUS input file.
[[nodiscard]] std::string read_abaqus_input_file(const std::filesystem::path &path);
} // namespace finelemethod::input
