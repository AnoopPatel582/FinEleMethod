#include "finelemethod/output/analysis_summary.hpp"

#include <nlohmann/json.hpp>

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace finelemethod::output
{
namespace
{
std::string required_string(const nlohmann::json &document, const std::string_view field_name)
{
    const auto field = document.find(field_name);
    if (field == document.end() || !field->is_string() ||
        field->get_ref<const std::string &>().empty())
    {
        throw std::invalid_argument("Analysis summary requires non-empty string field '" +
                                    std::string(field_name) + "'.");
    }
    return field->get<std::string>();
}

std::size_t required_size(const nlohmann::json &document, const std::string_view field_name)
{
    const auto field = document.find(field_name);
    if (field == document.end() || !field->is_number_unsigned())
    {
        throw std::invalid_argument("Analysis summary requires non-negative integer field '" +
                                    std::string(field_name) + "'.");
    }
    try
    {
        return field->get<std::size_t>();
    }
    catch (const nlohmann::json::out_of_range &)
    {
        throw std::invalid_argument("Analysis summary integer field is out of range: " +
                                    std::string(field_name));
    }
}
} // namespace

void write_analysis_summary(const std::filesystem::path &path, const AnalysisSummary &summary)
{
    const nlohmann::json document{
        {"protocolVersion", 1},
        {"status", "completed"},
        {"analysisType", summary.analysis_type},
        {"inputFile", summary.input_path.generic_string()},
        {"resultFile", summary.result_path.generic_string()},
        {"nodeCount", summary.node_count},
        {"elementCount", summary.element_count},
        {"solverIterations", summary.solver_iterations},
        {"residualNorm", summary.residual_norm},
    };

    std::ofstream stream(path);
    if (!stream)
    {
        throw std::runtime_error("could not open analysis summary file: " + path.string());
    }

    stream << document.dump(2) << '\n';
    if (!stream)
    {
        throw std::runtime_error("could not write analysis summary file: " + path.string());
    }
}

AnalysisSummary read_analysis_summary(const std::filesystem::path &path)
{
    std::ifstream stream(path);
    if (!stream)
    {
        throw std::runtime_error("could not open analysis summary file: " + path.string());
    }

    nlohmann::json document;
    try
    {
        stream >> document;
    }
    catch (const nlohmann::json::parse_error &exception)
    {
        throw std::invalid_argument("Analysis summary is not valid JSON: " +
                                    std::string(exception.what()));
    }

    if (!document.is_object())
    {
        throw std::invalid_argument("Analysis summary root must be a JSON object.");
    }
    const auto protocol_version = document.find("protocolVersion");
    if (protocol_version == document.end() || !protocol_version->is_number_integer() ||
        protocol_version->get<int>() != 1)
    {
        throw std::invalid_argument("Unsupported analysis summary protocolVersion.");
    }
    if (required_string(document, "status") != "completed")
    {
        throw std::invalid_argument("Analysis summary status must be 'completed'.");
    }

    const auto residual = document.find("residualNorm");
    if (residual == document.end() || !residual->is_number())
    {
        throw std::invalid_argument("Analysis summary requires numeric field 'residualNorm'.");
    }
    const double residual_norm = residual->get<double>();
    if (!std::isfinite(residual_norm) || residual_norm < 0.0)
    {
        throw std::invalid_argument(
            "Analysis summary residualNorm must be finite and non-negative.");
    }

    return AnalysisSummary{
        .analysis_type = required_string(document, "analysisType"),
        .input_path = required_string(document, "inputFile"),
        .result_path = required_string(document, "resultFile"),
        .node_count = required_size(document, "nodeCount"),
        .element_count = required_size(document, "elementCount"),
        .solver_iterations = required_size(document, "solverIterations"),
        .residual_norm = residual_norm,
    };
}
} // namespace finelemethod::output
