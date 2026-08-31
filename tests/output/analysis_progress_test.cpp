#include "finelemethod/output/analysis_progress.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <array>
#include <sstream>
#include <string>

namespace finelemethod::output
{
namespace
{
TEST(AnalysisProgress, WritesOneVersionedJsonObjectPerLine)
{
    std::ostringstream stream;

    write_analysis_progress_json_line(
        stream, AnalysisProgressEvent{AnalysisState::preparing, "Reading input model."});
    write_analysis_progress_json_line(
        stream, AnalysisProgressEvent{AnalysisState::executing, "Solving global system."});

    std::istringstream lines(stream.str());
    std::string first_line;
    std::string second_line;
    std::string third_line;
    ASSERT_TRUE(static_cast<bool>(std::getline(lines, first_line)));
    ASSERT_TRUE(static_cast<bool>(std::getline(lines, second_line)));
    EXPECT_FALSE(static_cast<bool>(std::getline(lines, third_line)));

    const auto first = nlohmann::json::parse(first_line);
    EXPECT_EQ(first.at("protocolVersion"), 1);
    EXPECT_EQ(first.at("event"), "analysis-progress");
    EXPECT_EQ(first.at("state"), "preparing");
    EXPECT_EQ(first.at("message"), "Reading input model.");

    const auto second = nlohmann::json::parse(second_line);
    EXPECT_EQ(second.at("protocolVersion"), 1);
    EXPECT_EQ(second.at("event"), "analysis-progress");
    EXPECT_EQ(second.at("state"), "executing");
    EXPECT_EQ(second.at("message"), "Solving global system.");
}

TEST(AnalysisProgress, SerializesEveryLifecycleState)
{
    const std::array expected_states{"preparing", "executing", "writing-results",
                                     "completed", "failed",    "cancelled"};
    const std::array states{
        AnalysisState::preparing, AnalysisState::executing, AnalysisState::writing_results,
        AnalysisState::completed, AnalysisState::failed,    AnalysisState::cancelled,
    };

    for (std::size_t index = 0; index < states.size(); ++index)
    {
        std::ostringstream stream;
        write_analysis_progress_json_line(stream, AnalysisProgressEvent{states[index], "message"});
        const auto document = nlohmann::json::parse(stream.str());
        EXPECT_EQ(document.at("state"), expected_states[index]);
    }
}

TEST(AnalysisProgress, ReportsAnOutputStreamFailure)
{
    std::ostringstream stream;
    stream.setstate(std::ios::badbit);

    EXPECT_THROW(write_analysis_progress_json_line(
                     stream, AnalysisProgressEvent{AnalysisState::failed, "failure"}),
                 std::runtime_error);
}
} // namespace
} // namespace finelemethod::output
