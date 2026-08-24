#include "finelemethod/cli/command_line.hpp"

#include <gtest/gtest.h>

#include <array>
#include <sstream>
#include <string>
#include <string_view>

TEST(CommandLine, HelpWritesUsageAndSucceeds)
{
    constexpr std::array<std::string_view, 1> arguments{"--help"};
    std::ostringstream output;
    std::ostringstream error;

    const auto exit_code = finelemethod::cli::run(arguments, output, error);

    EXPECT_EQ(exit_code, finelemethod::ExitCode::Success);
    EXPECT_NE(output.str().find("Usage:"), std::string::npos);
    EXPECT_TRUE(error.str().empty());
}

TEST(CommandLine, MissingArgumentWritesUsageError)
{
    constexpr std::array<std::string_view, 0> arguments{};
    std::ostringstream output;
    std::ostringstream error;

    const auto exit_code = finelemethod::cli::run(arguments, output, error);

    EXPECT_EQ(exit_code, finelemethod::ExitCode::UsageError);
    EXPECT_TRUE(output.str().empty());
    EXPECT_NE(error.str().find("analysis request is required"), std::string::npos);
    EXPECT_NE(error.str().find("Usage:"), std::string::npos);
}

TEST(CommandLine, UnknownOptionWritesUsageError)
{
    constexpr std::array<std::string_view, 1> arguments{"--unknown"};
    std::ostringstream output;
    std::ostringstream error;

    const auto exit_code = finelemethod::cli::run(arguments, output, error);

    EXPECT_EQ(exit_code, finelemethod::ExitCode::UsageError);
    EXPECT_TRUE(output.str().empty());
    EXPECT_NE(error.str().find("unknown command-line argument"), std::string::npos);
}
