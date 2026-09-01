#include "finelemethod/project/cancellation_flag.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace finelemethod::project
{
namespace
{
class CancellationFlagTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        run_directory_ =
            std::filesystem::temp_directory_path() / "finelemethod-cancellation-flag-test";
        std::filesystem::remove_all(run_directory_);
        std::filesystem::create_directory(run_directory_);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(run_directory_);
    }

    std::filesystem::path run_directory_;
};

TEST_F(CancellationFlagTest, UsesStableRunLocalFilename)
{
    EXPECT_EQ(cancellation_flag_path(run_directory_),
              run_directory_ / "cancellation-requested.flag");
}

TEST_F(CancellationFlagTest, DetectsRequestedCancellation)
{
    EXPECT_FALSE(is_analysis_cancellation_requested(run_directory_));

    request_analysis_cancellation(run_directory_);

    EXPECT_TRUE(is_analysis_cancellation_requested(run_directory_));
}

TEST_F(CancellationFlagTest, RepeatedRequestsRemainValid)
{
    request_analysis_cancellation(run_directory_);
    request_analysis_cancellation(run_directory_);

    EXPECT_TRUE(is_analysis_cancellation_requested(run_directory_));
}

TEST_F(CancellationFlagTest, RejectsMissingRunDirectory)
{
    std::filesystem::remove_all(run_directory_);

    EXPECT_THROW(request_analysis_cancellation(run_directory_), std::invalid_argument);
}
} // namespace
} // namespace finelemethod::project
