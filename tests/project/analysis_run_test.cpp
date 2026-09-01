#include "finelemethod/project/analysis_run.hpp"

#include "finelemethod/input/analysis_request.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace finelemethod::project
{
namespace
{
class AnalysisRunTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        root_ = std::filesystem::temp_directory_path() / "finelemethod-analysis-run-test";
        std::filesystem::remove_all(root_);
        std::filesystem::create_directory(root_);
        source_input_ = root_ / "source-model.inp";
        std::ofstream(source_input_) << "*HEADING\nOriginal model\n";
        project_ = create_project(root_, "BracketStudy", source_input_);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(root_);
    }

    std::filesystem::path root_;
    std::filesystem::path source_input_;
    ProjectFile project_;
};

TEST_F(AnalysisRunTest, CreatesFirstRunSnapshotAndRequest)
{
    const AnalysisRun run = prepare_analysis_run(project_);

    EXPECT_EQ(run.number, 1);
    EXPECT_EQ(run.run_directory.filename(), "run-0001");
    EXPECT_TRUE(std::filesystem::is_regular_file(run.input_file));
    EXPECT_TRUE(std::filesystem::is_directory(run.result_file.parent_path()));

    const input::AnalysisRequest request = input::read_analysis_request(run.request_file);
    EXPECT_EQ(request.input_file, std::filesystem::path("input/source-model.inp"));
    EXPECT_EQ(request.result_file, std::filesystem::path("results/source-model.vtu"));
    EXPECT_EQ(request.summary_file, std::filesystem::path("results/analysis-summary.json"));
}

TEST_F(AnalysisRunTest, CreatesMonotonicallyNumberedRunsWithoutChangingEarlierSnapshot)
{
    const AnalysisRun first = prepare_analysis_run(project_);
    std::ofstream(project_.input_file, std::ios::trunc) << "*HEADING\nUpdated model\n";
    const AnalysisRun second = prepare_analysis_run(project_);

    EXPECT_EQ(second.number, 2);
    EXPECT_EQ(second.run_directory.filename(), "run-0002");
    EXPECT_NE(first.run_directory, second.run_directory);

    std::ifstream first_input(first.input_file);
    std::string first_text((std::istreambuf_iterator<char>(first_input)),
                           std::istreambuf_iterator<char>());
    EXPECT_EQ(first_text, "*HEADING\nOriginal model\n");
}

TEST_F(AnalysisRunTest, IgnoresUnrelatedDirectoriesWhenChoosingNextNumber)
{
    std::filesystem::create_directory(project_.runs_directory / "notes");
    std::filesystem::create_directory(project_.runs_directory / "run-draft");

    const AnalysisRun run = prepare_analysis_run(project_);

    EXPECT_EQ(run.number, 1);
    EXPECT_EQ(run.run_directory.filename(), "run-0001");
}

TEST_F(AnalysisRunTest, RejectsMissingAuthoritativeInput)
{
    std::filesystem::remove(project_.input_file);

    EXPECT_THROW((void)prepare_analysis_run(project_), std::invalid_argument);
    EXPECT_TRUE(std::filesystem::is_empty(project_.runs_directory));
}
} // namespace
} // namespace finelemethod::project
