#pragma once

#include <filesystem>
#include <string>

namespace finelemethod::project
{
struct ProjectFile
{
    std::string name;
    std::filesystem::path project_directory;
    std::filesystem::path project_file;
    std::filesystem::path input_file;
    std::filesystem::path runs_directory;
};

// Creates a new file-based project without overwriting an existing directory.
// The imported ABAQUS file is copied into the project's input directory.
[[nodiscard]] ProjectFile create_project(const std::filesystem::path &parent_directory,
                                         const std::string &project_name,
                                         const std::filesystem::path &abaqus_input_file);

// Atomically writes the project JSON and retains the previous file as .bak.
// Recreates a missing main JSON without replacing any existing .bak file.
void save_project_file(const ProjectFile &project);

// Returns the separate autosave path beside the authoritative project file.
[[nodiscard]] std::filesystem::path project_autosave_path(const ProjectFile &project);

// Atomically writes, validates, and removes the separate autosave snapshot.
void write_project_autosave(const ProjectFile &project);
[[nodiscard]] ProjectFile read_project_autosave(const ProjectFile &project);
// Reads a snapshot without parsing the possibly damaged authoritative JSON.
// The snapshot identity, input path, and on-disk project structure are validated.
[[nodiscard]] ProjectFile read_project_autosave(const std::filesystem::path &project_file);
void remove_project_autosave(const ProjectFile &project);

// Reads and validates an existing protocol-version 1 FinEleMethod project.
[[nodiscard]] ProjectFile read_project_file(const std::filesystem::path &project_file);
} // namespace finelemethod::project
