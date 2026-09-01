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
class FlushTrackingBuffer final : public std::stringbuf
{
  public:
    int sync() override
    {
        ++flush_count;
        return std::stringbuf::sync();
    }

    int flush_count{};
};

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

TEST(AnalysisProgress, FlushesEachRecordForRealtimeConsumers)
{
    FlushTrackingBuffer buffer;
    std::ostream stream(&buffer);

    write_analysis_progress_json_line(
        stream, AnalysisProgressEvent{AnalysisState::executing, "Solving model."});

    EXPECT_EQ(buffer.flush_count, 1);
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

TEST(AnalysisProgress, ParsesEveryLifecycleState)
{
    const std::array states{
        AnalysisState::preparing, AnalysisState::executing, AnalysisState::writing_results,
        AnalysisState::completed, AnalysisState::failed,    AnalysisState::cancelled,
    };

    for (const AnalysisState state : states)
    {
        std::ostringstream stream;
        write_analysis_progress_json_line(stream, AnalysisProgressEvent{state, "Status message"});

        const AnalysisProgressEvent event = parse_analysis_progress_json_line(stream.str());

        EXPECT_EQ(event.state, state);
        EXPECT_EQ(event.message, "Status message");
    }
}

TEST(AnalysisProgress, ParserRejectsMalformedJson)
{
    EXPECT_THROW((void)parse_analysis_progress_json_line("{invalid"), std::invalid_argument);
}

TEST(AnalysisProgress, ParserRejectsUnsupportedVersionAndEventType)
{
    EXPECT_THROW(
        (void)parse_analysis_progress_json_line(
            R"({"protocolVersion":2,"event":"analysis-progress","state":"executing","message":"Solving"})"),
        std::invalid_argument);
    EXPECT_THROW(
        (void)parse_analysis_progress_json_line(
            R"({"protocolVersion":1,"event":"other","state":"executing","message":"Solving"})"),
        std::invalid_argument);
}

TEST(AnalysisProgress, ParserRejectsUnknownStateAndMissingMessage)
{
    EXPECT_THROW(
        (void)parse_analysis_progress_json_line(
            R"({"protocolVersion":1,"event":"analysis-progress","state":"waiting","message":"Waiting"})"),
        std::invalid_argument);
    EXPECT_THROW((void)parse_analysis_progress_json_line(
                     R"({"protocolVersion":1,"event":"analysis-progress","state":"executing"})"),
                 std::invalid_argument);
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
