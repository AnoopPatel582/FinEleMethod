#include "../../apps/gui/analysis_completion.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <stdexcept>
#include <system_error>
#include <vector>

namespace finelemethod::gui
{
TEST(AnalysisCompletion, RestoresControlsBeforeRefreshingHistory)
{
    std::vector<int> calls;
    const auto warning =
        restore_after_analysis([&] { calls.push_back(1); }, [&] { calls.push_back(2); });
    EXPECT_FALSE(warning.has_value());
    EXPECT_EQ(calls, (std::vector<int>{1, 2}));
}

TEST(AnalysisCompletion, HistoryErrorDoesNotPreventOutcomeProcessing)
{
    bool controls_enabled = false;
    const auto warning =
        restore_after_analysis([&] { controls_enabled = true; },
                               [&] {
                                   EXPECT_TRUE(controls_enabled);
                                   throw std::runtime_error("Run history is unavailable");
                               });
    ASSERT_TRUE(warning.has_value());
    EXPECT_EQ(*warning, "Run history is unavailable");
    EXPECT_TRUE(controls_enabled);
    // Returning normally lets the caller validate the solver outcome separately.
}

TEST(AnalysisCompletion, FilesystemFailureRetainsItsDiagnostic)
{
    const std::filesystem::filesystem_error failure(
        "read run history", std::make_error_code(std::errc::permission_denied));
    const auto warning = restore_after_analysis([] {}, [&] { throw failure; });
    ASSERT_TRUE(warning.has_value());
    EXPECT_EQ(*warning, failure.what());
}

TEST(AnalysisCompletion, DoesNotMislabelControlRestorationFailureAsHistoryWarning)
{
    bool refreshed = false;
    EXPECT_THROW((void)restore_after_analysis([] { throw std::logic_error("control failure"); },
                                              [&] { refreshed = true; }),
                 std::logic_error);
    EXPECT_FALSE(refreshed);
}
} // namespace finelemethod::gui
