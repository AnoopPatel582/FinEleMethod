#include "finelemethod/project/project_file.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

namespace finelemethod::project
{
namespace
{
class ProjectFileTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        root_ = std::filesystem::temp_directory_path() / "finelemethod-project-file-test";
        std::filesystem::remove_all(root_);
        std::filesystem::create_directory(root_);
        input_file_ = root_ / "source-model.inp";
        std::ofstream(input_file_) << "*HEADING\nFinEleMethod project test\n";
    }

    void TearDown() override
    {
        std::filesystem::remove_all(root_);
    }

    std::filesystem::path root_;
    std::filesystem::path input_file_;
};

TEST_F(ProjectFileTest, CreatesProjectAndImportsAbaqusInput)
{
    const ProjectFile project = create_project(root_, "BracketStudy", input_file_);

    EXPECT_EQ(project.name, "BracketStudy");
    EXPECT_EQ(project.project_directory, root_ / "BracketStudy");
    EXPECT_TRUE(std::filesystem::is_directory(project.runs_directory));
    EXPECT_TRUE(std::filesystem::is_regular_file(project.input_file));

    std::ifstream imported(project.input_file);
    std::string imported_text((std::istreambuf_iterator<char>(imported)),
                              std::istreambuf_iterator<char>());
    EXPECT_EQ(imported_text, "*HEADING\nFinEleMethod project test\n");

    std::ifstream project_stream(project.project_file);
    const auto document = nlohmann::json::parse(project_stream);
    EXPECT_EQ(document.at("schemaVersion"), 1);
    EXPECT_EQ(document.at("name"), "BracketStudy");
    EXPECT_EQ(document.at("inputFile"), "input/source-model.inp");

    const ProjectFile reopened = read_project_file(project.project_file);
    EXPECT_EQ(reopened.name, project.name);
    EXPECT_EQ(reopened.project_directory, project.project_directory);
    EXPECT_EQ(reopened.project_file, project.project_file);
    EXPECT_EQ(reopened.input_file, project.input_file);
    EXPECT_EQ(reopened.runs_directory, project.runs_directory);
}

TEST_F(ProjectFileTest, ReaderRejectsMalformedOrUnsupportedProject)
{
    const auto directory = root_ / "BrokenProject";
    std::filesystem::create_directory(directory);
    const auto path = directory / "BrokenProject.json";

    std::ofstream(path) << "{ invalid";
    EXPECT_THROW((void)read_project_file(path), std::invalid_argument);

    std::ofstream(path, std::ios::trunc) << R"({"schemaVersion":2})";
    EXPECT_THROW((void)read_project_file(path), std::invalid_argument);
}

TEST_F(ProjectFileTest, ReaderRejectsInputPathOutsideProject)
{
    const auto directory = root_ / "UnsafeProject";
    std::filesystem::create_directories(directory / "runs");
    const auto path = directory / "UnsafeProject.json";
    std::ofstream(path) << R"({
  "schemaVersion": 1,
  "name": "UnsafeProject",
  "inputFile": "../source-model.inp"
})";

    EXPECT_THROW((void)read_project_file(path), std::invalid_argument);
}

TEST_F(ProjectFileTest, ReaderRejectsMissingProjectStructure)
{
    const auto directory = root_ / "IncompleteProject";
    std::filesystem::create_directory(directory);
    const auto path = directory / "IncompleteProject.json";
    std::ofstream(path) << R"({
  "schemaVersion": 1,
  "name": "IncompleteProject",
  "inputFile": "input/model.inp"
})";

    EXPECT_THROW((void)read_project_file(path), std::invalid_argument);
}

TEST_F(ProjectFileTest, ReaderReportsMissingProjectFile)
{
    EXPECT_THROW((void)read_project_file(root_ / "Missing.json"), std::runtime_error);
}

TEST_F(ProjectFileTest, RefusesToOverwriteExistingProjectDirectory)
{
    std::filesystem::create_directory(root_ / "ExistingProject");

    EXPECT_THROW((void)create_project(root_, "ExistingProject", input_file_),
                 std::invalid_argument);
}

TEST_F(ProjectFileTest, RejectsUnsafeProjectName)
{
    EXPECT_THROW((void)create_project(root_, "../Outside", input_file_), std::invalid_argument);
    EXPECT_FALSE(std::filesystem::exists(root_.parent_path() / "Outside"));
}

TEST_F(ProjectFileTest, RejectsMissingAbaqusInput)
{
    EXPECT_THROW((void)create_project(root_, "MissingInput", root_ / "missing.inp"),
                 std::invalid_argument);
    EXPECT_FALSE(std::filesystem::exists(root_ / "MissingInput"));
}
} // namespace
} // namespace finelemethod::project
