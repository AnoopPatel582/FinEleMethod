#include "finelemethod/output/analysis_summary.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

namespace finelemethod::output
{
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
} // namespace finelemethod::output
