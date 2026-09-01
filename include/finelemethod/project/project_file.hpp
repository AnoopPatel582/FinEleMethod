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
} // namespace finelemethod::project
