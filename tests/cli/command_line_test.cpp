#include "finelemethod/cli/command_line.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <array>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>

namespace
{
constexpr std::string_view valid_abaqus_input = R"(*Node
1, 0.0, 0.0
2, 1.0, 0.0
3, 1.0, 1.0
4, 0.0, 1.0
*Material, name=TestMaterial
*Elastic
1000.0, 0.25
*Element, type=CPS4, elset=plate
1, 1, 2, 3, 4
*Solid Section, elset=plate, material=TestMaterial
1.0
*Boundary
1, 1, 2
4, 1
*Cload
2, 1, 5.0
3, 1, 5.0
)";

constexpr std::string_view valid_h8_abaqus_input = R"(*Node
1,0,0,0
2,1,0,0
3,1,1,0
4,0,1,0
5,0,0,1
6,1,0,1
7,1,1,1
8,0,1,1
*Material, name=TestMaterial
*Elastic
1000,0.25
*Element, type=C3D8, elset=block
1,1,2,3,4,5,6,7,8
*Solid Section, elset=block, material=TestMaterial
*Nset, nset=bottom
1,2,3,4
*Nset, nset=xzero
1,4,5,8
*Nset, nset=yzero
1,2,5,6
*Nset, nset=top
5,6,7,8
*Boundary
bottom,3
xzero,1
yzero,2
*Cload
top,3,-2.5
)";
} // namespace

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

TEST(CommandLine, H8CompressionExampleWritesParaViewResult)
{
    const auto path =
        std::filesystem::temp_directory_path() / "finelemethod_cli_h8_compression_test.vtu";
    std::filesystem::remove(path);
    const std::string path_text = path.string();
    const std::array<std::string_view, 4> arguments{"--example", "h8-compression", "--output",
                                                    path_text};
    std::ostringstream output;
    std::ostringstream error;

    const auto exit_code = finelemethod::cli::run(arguments, output, error);

    EXPECT_EQ(exit_code, finelemethod::ExitCode::Success);
    EXPECT_TRUE(error.str().empty());
    EXPECT_NE(output.str().find("Completed H8 compression example"), std::string::npos);
    std::ifstream file(path, std::ios::binary);
    const std::string vtu{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    EXPECT_NE(vtu.find("<Piece NumberOfPoints=\"8\" NumberOfCells=\"1\">"), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"types\" format=\"ascii\">\n          12"), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"Displacement\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"ReactionForce\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"Stress\" NumberOfComponents=\"6\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"VonMises\""), std::string::npos);
    file.close();
    std::filesystem::remove(path);
}

TEST(CommandLine, AbaqusInputWritesSolvedParaViewResult)
{
    const auto directory = std::filesystem::temp_directory_path();
    const auto input_path = directory / "finelemethod_cli_input_test.inp";
    const auto output_path = directory / "finelemethod_cli_input_test.vtu";
    {
        std::ofstream input_file(input_path, std::ios::binary);
        input_file << valid_abaqus_input;
    }
    std::filesystem::remove(output_path);
    const std::string input_text = input_path.string();
    const std::string output_text = output_path.string();
    const std::array<std::string_view, 4> arguments{"--input", input_text, "--output", output_text};
    std::ostringstream output;
    std::ostringstream error;

    const auto exit_code = finelemethod::cli::run(arguments, output, error);

    EXPECT_EQ(exit_code, finelemethod::ExitCode::Success);
    EXPECT_TRUE(error.str().empty());
    EXPECT_NE(output.str().find("Completed ABAQUS Q4 plane-stress analysis"), std::string::npos);
    std::ifstream result_file(output_path, std::ios::binary);
    const std::string vtu{std::istreambuf_iterator<char>(result_file),
                          std::istreambuf_iterator<char>()};
    EXPECT_NE(vtu.find("Name=\"Displacement\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"VonMises\""), std::string::npos);
    result_file.close();
    std::filesystem::remove(input_path);
    std::filesystem::remove(output_path);
}

TEST(CommandLine, AbaqusInputWritesVersionedAnalysisSummary)
{
    const auto directory = std::filesystem::temp_directory_path();
    const auto input_path = directory / "finelemethod_cli_summary_input.inp";
    const auto output_path = directory / "finelemethod_cli_summary_result.vtu";
    const auto summary_path = directory / "finelemethod_cli_summary.json";
    {
        std::ofstream input_file(input_path, std::ios::binary);
        input_file << valid_abaqus_input;
    }
    std::filesystem::remove(output_path);
    std::filesystem::remove(summary_path);
    const std::string input_text = input_path.string();
    const std::string output_text = output_path.string();
    const std::string summary_text = summary_path.string();
    const std::array<std::string_view, 6> arguments{"--input",   input_text,  "--output",
                                                    output_text, "--summary", summary_text};
    std::ostringstream output;
    std::ostringstream error;

    const auto exit_code = finelemethod::cli::run(arguments, output, error);

    EXPECT_EQ(exit_code, finelemethod::ExitCode::Success);
    EXPECT_TRUE(error.str().empty());
    EXPECT_NE(output.str().find("Analysis summary:"), std::string::npos);
    std::ifstream summary_file(summary_path);
    const auto document = nlohmann::json::parse(summary_file);
    EXPECT_EQ(document.at("protocolVersion"), 1);
    EXPECT_EQ(document.at("status"), "completed");
    EXPECT_EQ(document.at("analysisType"), "q4-plane-stress");
    EXPECT_EQ(document.at("inputFile"), input_path.generic_string());
    EXPECT_EQ(document.at("resultFile"), output_path.generic_string());
    EXPECT_EQ(document.at("nodeCount"), 4);
    EXPECT_EQ(document.at("elementCount"), 1);

    summary_file.close();
    std::filesystem::remove(input_path);
    std::filesystem::remove(output_path);
    std::filesystem::remove(summary_path);
}

TEST(CommandLine, AbaqusCpe4InputAutomaticallyWritesPlaneStrainResult)
{
    const auto directory = std::filesystem::temp_directory_path();
    const auto input_path = directory / "finelemethod_cli_cpe4_input_test.inp";
    const auto output_path = directory / "finelemethod_cli_cpe4_input_test.vtu";
    std::string plane_strain_input(valid_abaqus_input);
    plane_strain_input.replace(plane_strain_input.find("CPS4"), 4, "CPE4");
    {
        std::ofstream input_file(input_path, std::ios::binary);
        input_file << plane_strain_input;
    }
    std::filesystem::remove(output_path);
    const std::string input_text = input_path.string();
    const std::string output_text = output_path.string();
    const std::array<std::string_view, 4> arguments{"--input", input_text, "--output", output_text};
    std::ostringstream output;
    std::ostringstream error;

    const auto exit_code = finelemethod::cli::run(arguments, output, error);

    EXPECT_EQ(exit_code, finelemethod::ExitCode::Success);
    EXPECT_TRUE(error.str().empty());
    EXPECT_NE(output.str().find("Completed ABAQUS Q4 plane-strain analysis"), std::string::npos);
    std::ifstream result_file(output_path, std::ios::binary);
    const std::string vtu{std::istreambuf_iterator<char>(result_file),
                          std::istreambuf_iterator<char>()};
    EXPECT_NE(vtu.find("Name=\"Displacement\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"Strain\" NumberOfComponents=\"4\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"Stress\" NumberOfComponents=\"4\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"VonMises\""), std::string::npos);
    result_file.close();
    std::filesystem::remove(input_path);
    std::filesystem::remove(output_path);
}

TEST(CommandLine, AbaqusC3d8InputAutomaticallyWritesH8Result)
{
    const auto directory = std::filesystem::temp_directory_path();
    const auto input_path = directory / "finelemethod_cli_c3d8_input_test.inp";
    const auto output_path = directory / "finelemethod_cli_c3d8_input_test.vtu";
    {
        std::ofstream input_file(input_path, std::ios::binary);
        input_file << valid_h8_abaqus_input;
    }
    std::filesystem::remove(output_path);
    const std::string input_text = input_path.string();
    const std::string output_text = output_path.string();
    const std::array<std::string_view, 4> arguments{"--input", input_text, "--output", output_text};
    std::ostringstream output;
    std::ostringstream error;

    const auto exit_code = finelemethod::cli::run(arguments, output, error);

    EXPECT_EQ(exit_code, finelemethod::ExitCode::Success);
    EXPECT_TRUE(error.str().empty());
    EXPECT_NE(output.str().find("Completed ABAQUS H8 analysis"), std::string::npos);
    std::ifstream result_file(output_path, std::ios::binary);
    const std::string vtu{std::istreambuf_iterator<char>(result_file),
                          std::istreambuf_iterator<char>()};
    EXPECT_NE(vtu.find("NumberOfPoints=\"8\" NumberOfCells=\"1\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"types\" format=\"ascii\">\n          12"), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"Stress\" NumberOfComponents=\"6\""), std::string::npos);
    EXPECT_NE(vtu.find("Name=\"VonMises\""), std::string::npos);
    result_file.close();
    std::filesystem::remove(input_path);
    std::filesystem::remove(output_path);
}

TEST(CommandLine, AbaqusInputReportsMissingInputFile)
{
    const auto input_path =
        std::filesystem::temp_directory_path() / "finelemethod_missing_cli_input.inp";
    const auto output_path =
        std::filesystem::temp_directory_path() / "finelemethod_missing_cli_input.vtu";
    std::filesystem::remove(input_path);
    const std::string input_text = input_path.string();
    const std::string output_text = output_path.string();
    const std::array<std::string_view, 4> arguments{"--input", input_text, "--output", output_text};
    std::ostringstream output;
    std::ostringstream error;

    const auto exit_code = finelemethod::cli::run(arguments, output, error);

    EXPECT_EQ(exit_code, finelemethod::ExitCode::InputParsingError);
    EXPECT_TRUE(output.str().empty());
    EXPECT_NE(error.str().find("Input-file error"), std::string::npos);
}
