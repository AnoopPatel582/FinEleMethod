#pragma once

#include <iosfwd>
#include <string>
#include <string_view>

namespace finelemethod::output
{
enum class AnalysisState
{
    preparing,
    executing,
    writing_results,
    completed,
    failed,
    cancelled,
};

struct AnalysisProgressEvent
{
    AnalysisState state;
    std::string message;
};

[[nodiscard]] std::string_view analysis_state_name(AnalysisState state);
[[nodiscard]] AnalysisState parse_analysis_state(std::string_view name);

// Writes and flushes one complete protocol-versioned JSON object followed by a newline.
void write_analysis_progress_json_line(std::ostream &stream, const AnalysisProgressEvent &event);

// Parses and validates one complete protocol-versioned JSON Lines record.
[[nodiscard]] AnalysisProgressEvent parse_analysis_progress_json_line(std::string_view line);
} // namespace finelemethod::output
