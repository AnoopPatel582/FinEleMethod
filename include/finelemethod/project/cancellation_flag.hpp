#pragma once

#include <filesystem>

namespace finelemethod::project
{
// Returns the stable cancellation flag location for an analysis run.
[[nodiscard]] std::filesystem::path cancellation_flag_path(
    const std::filesystem::path &run_directory);

// Atomically requests cancellation by creating the run's flag file.
void request_analysis_cancellation(const std::filesystem::path &run_directory);

// Checks whether cancellation has been requested without modifying the run.
[[nodiscard]] bool is_analysis_cancellation_requested(const std::filesystem::path &run_directory);
} // namespace finelemethod::project
