#include "finelemethod/cli/command_line.hpp"

#include <gtest/gtest.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iterator>
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

TEST(CommandLine, Q4TensionExampleWritesParaViewResult)
{
    const auto path =
        std::filesystem::temp_directory_path() / "finelemethod_cli_q4_tension_test.vtu";
    std::filesystem::remove(path);
    const std::string path_text = path.string();
    const std::array<std::string_view, 4> arguments{"--example", "q4-tension", "--output",
                                                    path_text};
    std::ostringstream output;
    std::ostringstream error;

    const auto exit_code = finelemethod::cli::run(arguments, output, error);

    EXPECT_EQ(exit_code, finelemethod::ExitCode::Success);
    EXPECT_TRUE(error.str().empty());
    EXPECT_NE(output.str().find("Completed Q4 tension example"), std::string::npos);
    std::ifstream file(path, std::ios::binary);
    const std::string vtu{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    EXPECT_NE(vtu.find("<VTKFile type=\"UnstructuredGrid\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"Displacement\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"ReactionForce\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"VonMises\""), std::string::npos);
    file.close();
    std::filesystem::remove(path);
}

TEST(CommandLine, Q4TensionExampleReportsResultWritingFailure)
{
    const auto missing_directory = std::filesystem::temp_directory_path() /
                                   "finelemethod_missing_cli_directory" / "result.vtu";
    std::filesystem::remove_all(missing_directory.parent_path());
    const std::string path_text = missing_directory.string();
    const std::array<std::string_view, 4> arguments{"--example", "q4-tension", "--output",
                                                    path_text};
    std::ostringstream output;
    std::ostringstream error;

    const auto exit_code = finelemethod::cli::run(arguments, output, error);

    EXPECT_EQ(exit_code, finelemethod::ExitCode::ResultWritingError);
    EXPECT_TRUE(output.str().empty());
    EXPECT_NE(error.str().find("Result-writing error"), std::string::npos);
}
