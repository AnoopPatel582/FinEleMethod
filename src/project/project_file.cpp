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

std::filesystem::path read_relative_input_path(const nlohmann::json &document)
{
    const auto field = document.find("inputFile");
    if (field == document.end() || !field->is_string())
    {
        throw std::invalid_argument("Project file requires string field 'inputFile'.");
    }

    const std::filesystem::path path{field->get<std::string>()};
    if (path.empty() || path.is_absolute() || path.has_root_name() || path.has_root_directory())
    {
        throw std::invalid_argument("Project inputFile must be a non-empty relative path.");
    }
    for (const auto &component : path)
    {
        if (component == "..")
        {
            throw std::invalid_argument("Project inputFile must not escape the project directory.");
        }
    }
    if (path.begin() == path.end() || *path.begin() != "input" || path.extension() != ".inp")
    {
        throw std::invalid_argument("Project inputFile must identify an .inp file inside input/.");
    }
    return path;
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

ProjectFile read_project_file(const std::filesystem::path &project_file)
{
    std::ifstream stream(project_file);
    if (!stream)
    {
        throw std::runtime_error("Could not open project file: " + project_file.string());
    }

    nlohmann::json document;
    try
    {
        stream >> document;
    }
    catch (const nlohmann::json::parse_error &exception)
    {
        throw std::invalid_argument("Project file is not valid JSON: " +
                                    std::string(exception.what()));
    }
    if (!document.is_object())
    {
        throw std::invalid_argument("Project file root must be a JSON object.");
    }

    const auto schema_version = document.find("schemaVersion");
    if (schema_version == document.end() || !schema_version->is_number_integer() ||
        schema_version->get<int>() != 1)
    {
        throw std::invalid_argument("Unsupported project schemaVersion.");
    }

    const auto name_field = document.find("name");
    if (name_field == document.end() || !name_field->is_string())
    {
        throw std::invalid_argument("Project file requires string field 'name'.");
    }
    const std::string name = name_field->get<std::string>();
    validate_project_name(name);
    if (project_file.filename() != std::filesystem::path{name + ".json"})
    {
        throw std::invalid_argument("Project file name must match its project name.");
    }

    const std::filesystem::path project_directory = project_file.parent_path();
    const std::filesystem::path input_file =
        (project_directory / read_relative_input_path(document)).lexically_normal();
    const std::filesystem::path runs_directory = project_directory / "runs";
    if (!std::filesystem::is_regular_file(input_file))
    {
        throw std::invalid_argument("Project input file does not exist: " + input_file.string());
    }
    if (!std::filesystem::is_directory(runs_directory))
    {
        throw std::invalid_argument("Project runs directory does not exist: " +
                                    runs_directory.string());
    }

    return ProjectFile{
        .name = name,
        .project_directory = project_directory,
        .project_file = project_file,
        .input_file = input_file,
        .runs_directory = runs_directory,
    };
}
} // namespace finelemethod::project
