#pragma once

#include <filesystem>

namespace finelemethod::input
{
struct AnalysisRequest
{
    std::filesystem::path input_file;
    std::filesystem::path result_file;
    std::filesystem::path summary_file;
};

// Reads and validates a protocol-version 1 analysis request. Paths remain
// relative and are resolved against the request-file directory by the caller.
[[nodiscard]] AnalysisRequest read_analysis_request(const std::filesystem::path &path);

// Writes a validated protocol-version 1 analysis request. The destination's
// parent directory must already exist.
void write_analysis_request(const std::filesystem::path &path, const AnalysisRequest &request);
} // namespace finelemethod::input
