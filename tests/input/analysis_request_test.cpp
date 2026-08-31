#include "finelemethod/input/analysis_request.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string_view>

namespace finelemethod::input
{
namespace
{
class AnalysisRequestTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        path_ = std::filesystem::temp_directory_path() / "finelemethod-analysis-request.json";
        std::filesystem::remove(path_);
    }

    void TearDown() override
    {
        std::filesystem::remove(path_);
    }

    void write(const std::string_view text) const
    {
        std::ofstream stream(path_);
        stream << text;
    }

    std::filesystem::path path_;
};

TEST_F(AnalysisRequestTest, ReadsVersionOneRelativePaths)
{
    write(R"({
  "protocolVersion": 1,
  "inputFile": "input/model.inp",
  "resultFile": "results/model.vtu",
  "summaryFile": "analysis-summary.json"
})");

    const AnalysisRequest request = read_analysis_request(path_);

    EXPECT_EQ(request.input_file, std::filesystem::path("input/model.inp"));
    EXPECT_EQ(request.result_file, std::filesystem::path("results/model.vtu"));
    EXPECT_EQ(request.summary_file, std::filesystem::path("analysis-summary.json"));
}

TEST_F(AnalysisRequestTest, RejectsInvalidJson)
{
    write("{ invalid json }");

    EXPECT_THROW((void)read_analysis_request(path_), std::invalid_argument);
}

TEST_F(AnalysisRequestTest, RejectsUnsupportedProtocolVersion)
{
    write(R"({
  "protocolVersion": 2,
  "inputFile": "model.inp",
  "resultFile": "model.vtu",
  "summaryFile": "summary.json"
})");

    EXPECT_THROW((void)read_analysis_request(path_), std::invalid_argument);
}

TEST_F(AnalysisRequestTest, RejectsMissingRequiredField)
{
    write(R"({
  "protocolVersion": 1,
  "inputFile": "model.inp",
  "resultFile": "model.vtu"
})");

    EXPECT_THROW((void)read_analysis_request(path_), std::invalid_argument);
}

TEST_F(AnalysisRequestTest, RejectsAbsolutePaths)
{
    write(R"({
  "protocolVersion": 1,
  "inputFile": "C:/models/model.inp",
  "resultFile": "model.vtu",
  "summaryFile": "summary.json"
})");

    EXPECT_THROW((void)read_analysis_request(path_), std::invalid_argument);
}

TEST_F(AnalysisRequestTest, RejectsParentDirectoryTraversal)
{
    write(R"({
  "protocolVersion": 1,
  "inputFile": "../model.inp",
  "resultFile": "model.vtu",
  "summaryFile": "summary.json"
})");

    EXPECT_THROW((void)read_analysis_request(path_), std::invalid_argument);
}

TEST_F(AnalysisRequestTest, ReportsMissingRequestFile)
{
    EXPECT_THROW((void)read_analysis_request(path_), std::runtime_error);
}
} // namespace
} // namespace finelemethod::input
