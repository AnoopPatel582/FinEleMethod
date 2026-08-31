#include "finelemethod/output/analysis_progress.hpp"

#include <nlohmann/json.hpp>

#include <ostream>
#include <stdexcept>
#include <string_view>

namespace finelemethod::output
{
namespace
{
std::string_view state_name(const AnalysisState state)
{
    switch (state)
    {
    case AnalysisState::preparing:
        return "preparing";
    case AnalysisState::executing:
        return "executing";
    case AnalysisState::writing_results:
        return "writing-results";
    case AnalysisState::completed:
        return "completed";
    case AnalysisState::failed:
        return "failed";
    case AnalysisState::cancelled:
        return "cancelled";
    }

    throw std::invalid_argument("Unknown analysis progress state.");
}
} // namespace

void write_analysis_progress_json_line(std::ostream &stream, const AnalysisProgressEvent &event)
{
    const nlohmann::json document{
        {"protocolVersion", 1},
        {"event", "analysis-progress"},
        {"state", state_name(event.state)},
        {"message", event.message},
    };

    stream << document.dump() << '\n';
    if (!stream)
    {
        throw std::runtime_error("could not write analysis progress event");
    }
}
} // namespace finelemethod::output
