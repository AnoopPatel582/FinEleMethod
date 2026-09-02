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

TEST_F(ProjectFileTest, SavesAtomicallyAndRetainsOnePreviousBackup)
{
    const ProjectFile project = create_project(root_, "SavedProject", input_file_);
    std::ofstream(project.project_file, std::ios::trunc) << R"({"previous":true})";

    save_project_file(project);

    const ProjectFile reopened = read_project_file(project.project_file);
    EXPECT_EQ(reopened.name, project.name);
    const std::filesystem::path backup{project.project_file.string() + ".bak"};
    ASSERT_TRUE(std::filesystem::is_regular_file(backup));
    std::ifstream backup_stream(backup);
    const auto backup_document = nlohmann::json::parse(backup_stream);
    EXPECT_TRUE(backup_document.at("previous"));

    save_project_file(project);
    std::ifstream latest_backup_stream(backup);
    const auto latest_backup = nlohmann::json::parse(latest_backup_stream);
    EXPECT_EQ(latest_backup.at("schemaVersion"), 1);
    EXPECT_EQ(latest_backup.at("name"), "SavedProject");
}

TEST_F(ProjectFileTest, SaveRejectsProjectFilePathThatDoesNotMatchItsName)
{
    ProjectFile project = create_project(root_, "MismatchedProject", input_file_);
    project.project_file = project.project_directory / "Other.json";

    EXPECT_THROW(save_project_file(project), std::invalid_argument);
}

TEST_F(ProjectFileTest, SaveAndAutosaveRejectMissingInputWithoutReplacingFiles)
{
    ProjectFile project = create_project(root_, "MissingInput", input_file_);
    write_project_autosave(project);
    const auto original_input = project.input_file;
    project.input_file = project.project_directory / "input" / "missing.inp";

    EXPECT_THROW(save_project_file(project), std::invalid_argument);
    EXPECT_THROW(write_project_autosave(project), std::invalid_argument);
    EXPECT_EQ(read_project_file(project.project_file).input_file, original_input);
    project.input_file = original_input;
    EXPECT_EQ(read_project_autosave(project).input_file, original_input);
    EXPECT_FALSE(std::filesystem::exists(project.project_file.string() + ".bak"));
    EXPECT_FALSE(std::filesystem::exists(project.project_file.string() + ".tmp"));
}

TEST_F(ProjectFileTest, SaveAndAutosaveRejectMissingOrMismatchedRunsDirectory)
{
    ProjectFile project = create_project(root_, "InvalidRuns", input_file_);
    const auto runs = project.runs_directory;
    project.runs_directory = root_;
    EXPECT_THROW(save_project_file(project), std::invalid_argument);
    EXPECT_THROW(write_project_autosave(project), std::invalid_argument);
    project.runs_directory = runs;
    std::filesystem::remove(runs);
    EXPECT_THROW(save_project_file(project), std::invalid_argument);
    EXPECT_THROW(write_project_autosave(project), std::invalid_argument);
    EXPECT_FALSE(std::filesystem::exists(project.project_file.string() + ".bak"));
    EXPECT_FALSE(std::filesystem::exists(project_autosave_path(project)));
}

TEST_F(ProjectFileTest, WritesReadsAndRemovesSeparateAutosave)
{
    const ProjectFile project = create_project(root_, "AutosavedProject", input_file_);
    const std::filesystem::path autosave = project_autosave_path(project);
    EXPECT_EQ(autosave, project.project_directory / "AutosavedProject.autosave.json");

    write_project_autosave(project);

    ASSERT_TRUE(std::filesystem::is_regular_file(autosave));
    EXPECT_FALSE(std::filesystem::exists(project.project_file.string() + ".bak"));
    const ProjectFile recovered = read_project_autosave(project);
    EXPECT_EQ(recovered.name, project.name);
    EXPECT_EQ(recovered.project_file, project.project_file);
    EXPECT_EQ(recovered.input_file, project.input_file);

    write_project_autosave(project);
    EXPECT_FALSE(std::filesystem::exists(autosave.string() + ".bak"));
    remove_project_autosave(project);
    EXPECT_FALSE(std::filesystem::exists(autosave));
    remove_project_autosave(project);
}

TEST_F(ProjectFileTest, AutosaveReaderRejectsMalformedSnapshot)
{
    const ProjectFile project = create_project(root_, "BrokenAutosave", input_file_);
    std::ofstream(project_autosave_path(project)) << "{ invalid";

    EXPECT_THROW((void)read_project_autosave(project), std::invalid_argument);
}

TEST_F(ProjectFileTest, RecoversSnapshotWhenAuthoritativeJsonIsCorruptWithoutWritingFiles)
{
    const ProjectFile project = create_project(root_, "Recovery", input_file_);
    write_project_autosave(project);
    std::ofstream(project.project_file, std::ios::trunc) << "{ invalid";

    const auto recovered = read_project_autosave(project.project_file);
    EXPECT_EQ(recovered.input_file, project.input_file);
    EXPECT_EQ(recovered.project_file, project.project_file);
    EXPECT_THROW((void)read_project_file(project.project_file), std::invalid_argument);
    EXPECT_TRUE(std::filesystem::exists(project_autosave_path(project)));
    EXPECT_FALSE(std::filesystem::exists(project.project_file.string() + ".bak"));
}

TEST_F(ProjectFileTest, PathBasedRecoveryRejectsMismatchedIdentityAndEscapingInput)
{
    const ProjectFile project = create_project(root_, "Recovery", input_file_);
    const auto snapshot = project_autosave_path(project);
    std::ofstream(snapshot)
        << R"({"schemaVersion":1,"name":"Other","inputFile":"input/source-model.inp"})";
    EXPECT_THROW((void)read_project_autosave(project.project_file), std::invalid_argument);
    std::ofstream(snapshot, std::ios::trunc)
        << R"({"schemaVersion":1,"name":"Recovery","inputFile":"../source-model.inp"})";
    EXPECT_THROW((void)read_project_autosave(project.project_file), std::invalid_argument);
    EXPECT_EQ(read_project_file(project.project_file).name, "Recovery");
}

TEST_F(ProjectFileTest, PathBasedRecoveryRejectsMissingSnapshotAndMissingInput)
{
    const ProjectFile project = create_project(root_, "Recovery", input_file_);
    EXPECT_THROW((void)read_project_autosave(project.project_file), std::runtime_error);
    write_project_autosave(project);
    std::filesystem::remove(project.input_file);
    EXPECT_THROW((void)read_project_autosave(project.project_file), std::invalid_argument);
    EXPECT_THROW((void)read_project_autosave(root_ / "Recovery.txt"), std::invalid_argument);
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
