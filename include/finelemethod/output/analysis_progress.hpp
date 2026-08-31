#pragma once

#include <iosfwd>
#include <string>

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

// Writes one complete protocol-versioned JSON object followed by a newline.
void write_analysis_progress_json_line(std::ostream &stream, const AnalysisProgressEvent &event);
} // namespace finelemethod::output
