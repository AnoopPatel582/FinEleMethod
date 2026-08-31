#include "finelemethod/input/analysis_request.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace finelemethod::input
{
namespace
{
std::filesystem::path read_relative_path(const nlohmann::json &document,
                                         const std::string_view field_name)
{
    const auto field = document.find(field_name);
    if (field == document.end() || !field->is_string())
    {
        throw std::invalid_argument("Analysis request requires string field '" +
                                    std::string(field_name) + "'.");
    }

    const std::filesystem::path path{field->get<std::string>()};
    if (path.empty())
    {
        throw std::invalid_argument("Analysis request field '" + std::string(field_name) +
                                    "' must not be empty.");
    }
    if (path.is_absolute() || path.has_root_name() || path.has_root_directory())
    {
        throw std::invalid_argument("Analysis request field '" + std::string(field_name) +
                                    "' must be a relative path.");
    }
    for (const auto &component : path)
    {
        if (component == "..")
        {
            throw std::invalid_argument("Analysis request field '" + std::string(field_name) +
                                        "' must not escape the request directory.");
        }
    }

    return path;
}
} // namespace

AnalysisRequest read_analysis_request(const std::filesystem::path &path)
{
    std::ifstream stream(path);
    if (!stream)
    {
        throw std::runtime_error("could not open analysis request file: " + path.string());
    }

    nlohmann::json document;
    try
    {
        stream >> document;
    }
    catch (const nlohmann::json::parse_error &exception)
    {
        throw std::invalid_argument("Analysis request is not valid JSON: " +
                                    std::string(exception.what()));
    }

    if (!document.is_object())
    {
        throw std::invalid_argument("Analysis request root must be a JSON object.");
    }

    const auto protocol_version = document.find("protocolVersion");
    if (protocol_version == document.end() || !protocol_version->is_number_integer())
    {
        throw std::invalid_argument("Analysis request requires integer field 'protocolVersion'.");
    }
    if (protocol_version->get<int>() != 1)
    {
        throw std::invalid_argument("Unsupported analysis request protocolVersion.");
    }

    return AnalysisRequest{
        .input_file = read_relative_path(document, "inputFile"),
        .result_file = read_relative_path(document, "resultFile"),
        .summary_file = read_relative_path(document, "summaryFile"),
    };
}
} // namespace finelemethod::input
