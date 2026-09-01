#pragma once

#include "finelemethod/project/project_file.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace finelemethod::project
{
struct AnalysisRun
{
    std::uint64_t number{};
    std::filesystem::path run_directory;
    std::filesystem::path request_file;
    std::filesystem::path input_file;
    std::filesystem::path result_file;
    std::filesystem::path summary_file;
};

// Creates the next numbered run directory and snapshots the authoritative
// project input. Existing run directories are never reused or overwritten.
[[nodiscard]] AnalysisRun prepare_analysis_run(const ProjectFile &project);

// Discovers prepared run directories and returns them in ascending run-number
// order. Unrelated entries in the project's runs directory are ignored.
[[nodiscard]] std::vector<AnalysisRun> list_analysis_runs(const ProjectFile &project);
} // namespace finelemethod::project
