#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

namespace finelemethod::output
{
struct AnalysisSummary
{
    std::string analysis_type;
    std::filesystem::path input_path;
    std::filesystem::path result_path;
    std::size_t node_count{};
    std::size_t element_count{};
};

void write_analysis_summary(const std::filesystem::path &path, const AnalysisSummary &summary);
} // namespace finelemethod::output
