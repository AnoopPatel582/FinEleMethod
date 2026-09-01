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

AnalysisState parse_state(const std::string_view name)
{
    if (name == "preparing")
    {
        return AnalysisState::preparing;
    }
    if (name == "executing")
    {
        return AnalysisState::executing;
    }
    if (name == "writing-results")
    {
        return AnalysisState::writing_results;
    }
    if (name == "completed")
    {
        return AnalysisState::completed;
    }
    if (name == "failed")
    {
        return AnalysisState::failed;
    }
    if (name == "cancelled")
    {
        return AnalysisState::cancelled;
    }
    throw std::invalid_argument("Unknown analysis progress state: " + std::string(name));
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

AnalysisProgressEvent parse_analysis_progress_json_line(const std::string_view line)
{
    nlohmann::json document;
    try
    {
        document = nlohmann::json::parse(line);
    }
    catch (const nlohmann::json::parse_error &exception)
    {
        throw std::invalid_argument("Analysis progress record is not valid JSON: " +
                                    std::string(exception.what()));
    }

    if (!document.is_object())
    {
        throw std::invalid_argument("Analysis progress record must be a JSON object.");
    }
    if (!document.contains("protocolVersion") ||
        !document.at("protocolVersion").is_number_integer() ||
        document.at("protocolVersion").get<int>() != 1)
    {
        throw std::invalid_argument("Unsupported analysis progress protocolVersion.");
    }
    if (!document.contains("event") || !document.at("event").is_string() ||
        document.at("event").get<std::string_view>() != "analysis-progress")
    {
        throw std::invalid_argument("Unsupported analysis progress event type.");
    }
    if (!document.contains("state") || !document.at("state").is_string())
    {
        throw std::invalid_argument("Analysis progress record requires string field 'state'.");
    }
    if (!document.contains("message") || !document.at("message").is_string())
    {
        throw std::invalid_argument("Analysis progress record requires string field 'message'.");
    }

    return AnalysisProgressEvent{
        .state = parse_state(document.at("state").get<std::string_view>()),
        .message = document.at("message").get<std::string>(),
    };
}
} // namespace finelemethod::output
