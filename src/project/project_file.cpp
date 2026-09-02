#include "finelemethod/project/project_file.hpp"

#include <nlohmann/json.hpp>

#include <Windows.h>

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

nlohmann::json project_document(const std::string &name,
                                const std::filesystem::path &relative_input_file)
{
    return nlohmann::json{
        {"schemaVersion", 1},
        {"name", name},
        {"inputFile", relative_input_file.generic_string()},
    };
}

std::filesystem::path write_temporary_project_file(const std::filesystem::path &path,
                                                   const nlohmann::json &document)
{
    const auto temporary_path = std::filesystem::path{path.string() + ".tmp"};
    {
        std::ofstream stream(temporary_path, std::ios::trunc);
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

    return temporary_path;
}

std::runtime_error windows_file_error(const std::string &operation, const DWORD error_code)
{
    const std::error_code error(static_cast<int>(error_code), std::system_category());
    return std::runtime_error(operation + ": " + error.message());
}

void install_new_project_file(const std::filesystem::path &path, const nlohmann::json &document)
{
    const std::filesystem::path temporary_path = write_temporary_project_file(path, document);

    if (!MoveFileExW(temporary_path.c_str(), path.c_str(), MOVEFILE_WRITE_THROUGH))
    {
        const DWORD error_code = GetLastError();
        std::filesystem::remove(temporary_path);
        throw windows_file_error("Could not finalize project file", error_code);
    }
}

void replace_project_file_with_backup(const std::filesystem::path &path,
                                      const nlohmann::json &document)
{
    const std::filesystem::path temporary_path = write_temporary_project_file(path, document);
    const std::filesystem::path backup_path{path.string() + ".bak"};
    if (!CopyFileW(path.c_str(), backup_path.c_str(), FALSE))
    {
        const DWORD error_code = GetLastError();
        std::filesystem::remove(temporary_path);
        throw windows_file_error("Could not write project backup", error_code);
    }

    if (!MoveFileExW(temporary_path.c_str(), path.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        const DWORD error_code = GetLastError();
        std::filesystem::remove(temporary_path);
        throw windows_file_error("Could not atomically save project file", error_code);
    }
}

void replace_file_without_backup(const std::filesystem::path &path, const nlohmann::json &document)
{
    const std::filesystem::path temporary_path = write_temporary_project_file(path, document);
    if (!MoveFileExW(temporary_path.c_str(), path.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        const DWORD error_code = GetLastError();
        std::filesystem::remove(temporary_path);
        throw windows_file_error("Could not atomically write autosave file", error_code);
    }
}

std::filesystem::path validate_project_for_save(const ProjectFile &project)
{
    validate_project_name(project.name);
    if (project.project_file != project.project_directory / (project.name + ".json"))
    {
        throw std::invalid_argument("Project file path does not match the project name.");
    }
    if (std::filesystem::exists(project.project_file) &&
        !std::filesystem::is_regular_file(project.project_file))
    {
        throw std::invalid_argument("Project file path is not a regular file: " +
                                    project.project_file.string());
    }

    const std::filesystem::path relative_input =
        project.input_file.lexically_relative(project.project_directory);
    const nlohmann::json document = project_document(project.name, relative_input);
    (void)read_relative_input_path(document);
    if (!std::filesystem::is_regular_file(project.input_file))
    {
        throw std::invalid_argument("Project input file does not exist: " +
                                    project.input_file.string());
    }
    if (project.runs_directory != project.project_directory / "runs" ||
        !std::filesystem::is_directory(project.runs_directory))
    {
        throw std::invalid_argument("Project runs directory must be the existing runs/ directory.");
    }
    return relative_input;
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
        install_new_project_file(project_file, project_document(project_name, relative_input_file));
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

void save_project_file(const ProjectFile &project)
{
    const std::filesystem::path relative_input = validate_project_for_save(project);
    const auto document = project_document(project.name, relative_input);
    if (std::filesystem::exists(project.project_file))
    {
        replace_project_file_with_backup(project.project_file, document);
    }
    else
    {
        // Recovery can recreate a missing main JSON. Do not replace a file that
        // appears concurrently, and leave any previous backup untouched.
        install_new_project_file(project.project_file, document);
    }
}

namespace
{
ProjectFile read_project_document(const std::filesystem::path &document_file,
                                  const std::filesystem::path &project_file)
{
    std::ifstream stream(document_file);
    if (!stream)
    {
        throw std::runtime_error("Could not open project file: " + document_file.string());
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
} // namespace

std::filesystem::path project_autosave_path(const ProjectFile &project)
{
    return project.project_directory / (project.name + ".autosave.json");
}

void write_project_autosave(const ProjectFile &project)
{
    const std::filesystem::path relative_input = validate_project_for_save(project);
    replace_file_without_backup(project_autosave_path(project),
                                project_document(project.name, relative_input));
}

ProjectFile read_project_autosave(const ProjectFile &project)
{
    if (project.project_file != project.project_directory / (project.name + ".json"))
    {
        throw std::invalid_argument("Project file path does not match the project name.");
    }
    return read_project_autosave(project.project_file);
}

ProjectFile read_project_autosave(const std::filesystem::path &project_file)
{
    if (project_file.extension() != ".json")
    {
        throw std::invalid_argument("Project file must have the .json extension.");
    }
    const std::string name = project_file.stem().string();
    validate_project_name(name);
    const auto autosave = project_file.parent_path() / (name + ".autosave.json");
    return read_project_document(autosave, project_file);
}

void remove_project_autosave(const ProjectFile &project)
{
    (void)validate_project_for_save(project);
    std::error_code error;
    std::filesystem::remove(project_autosave_path(project), error);
    if (error)
    {
        throw std::runtime_error("Could not remove project autosave: " + error.message());
    }
}

ProjectFile read_project_autosave_file(const std::filesystem::path &autosave_file)
{
    if (autosave_file.extension() != ".json" || autosave_file.stem().extension() != ".autosave")
    {
        throw std::invalid_argument("Select a <ProjectName>.autosave.json recovery snapshot.");
    }
    auto project_file = autosave_file;
    project_file.replace_extension();
    project_file.replace_extension(".json");
    return read_project_autosave(project_file);
}

ProjectFile read_project_file(const std::filesystem::path &project_file)
{
    return read_project_document(project_file, project_file);
}
} // namespace finelemethod::project
