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
    std::filesystem::remove(path);
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
