#include "finelemethod/output/analysis_summary.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

namespace finelemethod::output
{
namespace
{
TEST(AnalysisSummary, WritesVersionedCompletedAnalysisDocument)
{
    const auto path = std::filesystem::temp_directory_path() / "finelemethod-summary-test.json";
    const AnalysisSummary summary{
        .analysis_type = "q4-plane-stress",
        .input_path = "models/tension.inp",
        .result_path = "results/tension.vtu",
        .node_count = 4,
        .element_count = 1,
        .solver_iterations = 3,
        .residual_norm = 1.25e-12,
    };

    write_analysis_summary(path, summary);

    std::ifstream stream(path);
    const auto document = nlohmann::json::parse(stream);
    EXPECT_EQ(document.at("protocolVersion"), 1);
    EXPECT_EQ(document.at("status"), "completed");
    EXPECT_EQ(document.at("analysisType"), "q4-plane-stress");
    EXPECT_EQ(document.at("inputFile"), "models/tension.inp");
    EXPECT_EQ(document.at("resultFile"), "results/tension.vtu");
    EXPECT_EQ(document.at("nodeCount"), 4);
    EXPECT_EQ(document.at("elementCount"), 1);
    EXPECT_EQ(document.at("solverIterations"), 3);
    EXPECT_DOUBLE_EQ(document.at("residualNorm"), 1.25e-12);

    stream.close();
    const AnalysisSummary actual = read_analysis_summary(path);
    EXPECT_EQ(actual.analysis_type, summary.analysis_type);
    EXPECT_EQ(actual.input_path, summary.input_path);
    EXPECT_EQ(actual.result_path, summary.result_path);
    EXPECT_EQ(actual.node_count, summary.node_count);
    EXPECT_EQ(actual.element_count, summary.element_count);
    EXPECT_EQ(actual.solver_iterations, summary.solver_iterations);
    EXPECT_DOUBLE_EQ(actual.residual_norm, summary.residual_norm);

    std::filesystem::remove(path);
}

TEST(AnalysisSummary, ReaderRejectsMalformedOrIncompatibleDocument)
{
    const auto path = std::filesystem::temp_directory_path() / "finelemethod-summary-invalid.json";

    std::ofstream(path) << "{ invalid";
    EXPECT_THROW((void)read_analysis_summary(path), std::invalid_argument);

    std::ofstream(path, std::ios::trunc) << R"({"protocolVersion":2,"status":"completed"})";
    EXPECT_THROW((void)read_analysis_summary(path), std::invalid_argument);

    std::filesystem::remove(path);
}

TEST(AnalysisSummary, ReaderRejectsIncompleteOrFailedSummary)
{
    const auto path = std::filesystem::temp_directory_path() / "finelemethod-summary-invalid.json";

    std::ofstream(path) << R"({
  "protocolVersion": 1,
  "status": "failed",
  "analysisType": "q4-plane-stress",
  "inputFile": "model.inp",
  "resultFile": "model.vtu",
  "nodeCount": 4,
  "elementCount": 1,
  "solverIterations": 3,
  "residualNorm": 0.0
})";
    EXPECT_THROW((void)read_analysis_summary(path), std::invalid_argument);

    std::ofstream(path, std::ios::trunc) << R"({
  "protocolVersion": 1,
  "status": "completed",
  "analysisType": "q4-plane-stress",
  "inputFile": "model.inp",
  "resultFile": "model.vtu",
  "nodeCount": -1,
  "elementCount": 1,
  "solverIterations": 3,
  "residualNorm": 0.0
})";
    EXPECT_THROW((void)read_analysis_summary(path), std::invalid_argument);

    std::filesystem::remove(path);
}

TEST(AnalysisSummary, ReaderReportsMissingFile)
{
    const auto path = std::filesystem::temp_directory_path() / "finelemethod-summary-missing.json";
    std::filesystem::remove(path);

    EXPECT_THROW((void)read_analysis_summary(path), std::runtime_error);
}

TEST(AnalysisSummary, RejectsAnUnwritableDestination)
{
    const auto missing_parent = std::filesystem::temp_directory_path() /
                                "finelemethod-missing-summary-directory" / "summary.json";
    std::filesystem::remove_all(missing_parent.parent_path());

    EXPECT_THROW(write_analysis_summary(missing_parent, AnalysisSummary{}), std::runtime_error);
}
} // namespace
} // namespace finelemethod::output
