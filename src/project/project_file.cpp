#include "finelemethod/project/project_file.hpp"

#include <nlohmann/json.hpp>

#include <cctype>
#include <fstream>
#include <stdexcept>
#include <system_error>

namespace finelemethod::project
{
namespace
{
void validate_project_name(const std::string &name)
{
    if (name.empty())
    {
        throw std::invalid_argument("Project name must not be empty.");
    }

    for (const unsigned char character : name)
    {
        if (!(std::isalnum(character) || character == '-' || character == '_'))
        {
            throw std::invalid_argument(
                "Project name may contain only letters, numbers, hyphens, and underscores.");
        }
    }
}

void write_new_project_file(const std::filesystem::path &path, const std::string &name,
                            const std::filesystem::path &relative_input_file)
{
    const nlohmann::json document{
        {"schemaVersion", 1},
        {"name", name},
        {"inputFile", relative_input_file.generic_string()},
    };

    const auto temporary_path = std::filesystem::path{path.string() + ".tmp"};
    {
        std::ofstream stream(temporary_path);
        if (!stream)
        {
            throw std::runtime_error("Could not write temporary project file: " +
                                     temporary_path.string());
        }
        stream << document.dump(2) << '\n';
        if (!stream)
        {
            throw std::runtime_error("Could not write temporary project file: " +
                                     temporary_path.string());
        }
    }

    std::error_code error;
    std::filesystem::rename(temporary_path, path, error);
    if (error)
    {
        std::filesystem::remove(temporary_path);
        throw std::runtime_error("Could not finalize project file: " + error.message());
    }
}
} // namespace

ProjectFile create_project(const std::filesystem::path &parent_directory,
                           const std::string &project_name,
                           const std::filesystem::path &abaqus_input_file)
{
    validate_project_name(project_name);
    if (!std::filesystem::is_directory(parent_directory))
    {
        throw std::invalid_argument("Project parent directory does not exist: " +
                                    parent_directory.string());
    }
    if (!std::filesystem::is_regular_file(abaqus_input_file) ||
        abaqus_input_file.extension() != ".inp")
    {
        throw std::invalid_argument("ABAQUS input must be an existing .inp file: " +
                                    abaqus_input_file.string());
    }

    const auto project_directory = parent_directory / project_name;
    if (std::filesystem::exists(project_directory))
    {
        throw std::invalid_argument("Project directory already exists: " +
                                    project_directory.string());
    }

    const auto input_directory = project_directory / "input";
    const auto runs_directory = project_directory / "runs";
    const auto relative_input_file = std::filesystem::path{"input"} / abaqus_input_file.filename();
    const auto imported_input_file = project_directory / relative_input_file;
    const auto project_file = project_directory / (project_name + ".json");

    if (!std::filesystem::create_directory(project_directory))
    {
        throw std::invalid_argument("Project directory already exists: " +
                                    project_directory.string());
    }

    try
    {
        std::filesystem::create_directory(input_directory);
        std::filesystem::create_directory(runs_directory);
        std::filesystem::copy_file(abaqus_input_file, imported_input_file);
        write_new_project_file(project_file, project_name, relative_input_file);
    }
    catch (...)
    {
        std::error_code cleanup_error;
        std::filesystem::remove_all(project_directory, cleanup_error);
        throw;
    }

    return ProjectFile{
        .name = project_name,
        .project_directory = project_directory,
        .project_file = project_file,
        .input_file = imported_input_file,
        .runs_directory = runs_directory,
    };
}
} // namespace finelemethod::project
