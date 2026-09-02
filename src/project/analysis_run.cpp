#include "finelemethod/project/analysis_run.hpp"

#include "finelemethod/input/analysis_request.hpp"
#include "finelemethod/output/analysis_summary.hpp"

#include <nlohmann/json.hpp>

#define NOMINMAX
#include <Windows.h>

#include <algorithm>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace finelemethod::project
{
namespace
{
constexpr std::string_view run_prefix = "run-";
constexpr std::string_view state_filename = "analysis-state.json";

std::uint64_t run_number(const std::filesystem::path &directory)
{
    const std::string name = directory.filename().string();
    if (!name.starts_with(run_prefix))
    {
        return 0;
    }

    const std::string_view digits{name.data() + run_prefix.size(), name.size() - run_prefix.size()};
    if (digits.empty())
    {
        return 0;
    }

    std::uint64_t number{};
    const auto [end, error] = std::from_chars(digits.data(), digits.data() + digits.size(), number);
    if (error != std::errc{} || end != digits.data() + digits.size() || number == 0)
    {
        return 0;
    }
    return number;
}

std::string run_directory_name(const std::uint64_t number)
{
    std::ostringstream name;
    name << run_prefix << std::setfill('0') << std::setw(4) << number;
    return name.str();
}

AnalysisRun read_analysis_run(const std::filesystem::path &run_directory,
                              const std::uint64_t number)
{
    const auto request_file = run_directory / "analysis-request.json";
    const input::AnalysisRequest request = input::read_analysis_request(request_file);
    return AnalysisRun{
        .number = number,
        .run_directory = run_directory,
        .request_file = request_file,
        .input_file = run_directory / request.input_file,
        .result_file = run_directory / request.result_file,
        .summary_file = run_directory / request.summary_file,
        .state_file = run_directory / state_filename,
    };
}
} // namespace

std::vector<AnalysisRun> list_analysis_runs(const ProjectFile &project)
{
    if (!std::filesystem::is_directory(project.runs_directory))
    {
        throw std::invalid_argument("Project runs directory does not exist: " +
                                    project.runs_directory.string());
    }

    std::vector<AnalysisRun> runs;
    for (const auto &entry : std::filesystem::directory_iterator(project.runs_directory))
    {
        if (entry.is_directory())
        {
            const std::uint64_t number = run_number(entry.path());
            if (number != 0)
            {
                runs.push_back(read_analysis_run(entry.path(), number));
            }
        }
    }
    std::ranges::sort(runs, {}, &AnalysisRun::number);
    return runs;
}

bool is_analysis_run_completed(const AnalysisRun &run)
{
    try
    {
        const output::AnalysisSummary summary = output::read_analysis_summary(run.summary_file);
        return summary.input_path.lexically_normal() == run.input_file.lexically_normal() &&
               summary.result_path.lexically_normal() == run.result_file.lexically_normal() &&
               std::filesystem::is_regular_file(run.result_file);
    }
    catch (const std::exception &)
    {
        return false;
    }
}

void write_analysis_run_state(const std::filesystem::path &run_directory,
                              const output::AnalysisProgressEvent &event)
{
    if (!std::filesystem::is_directory(run_directory))
    {
        throw std::invalid_argument("Analysis run directory does not exist: " +
                                    run_directory.string());
    }

    const std::filesystem::path state_file = run_directory / state_filename;
    const std::filesystem::path temporary_file{state_file.string() + ".tmp"};
    const nlohmann::json document{
        {"schemaVersion", 1},
        {"state", output::analysis_state_name(event.state)},
        {"message", event.message},
    };
    {
        std::ofstream stream(temporary_file, std::ios::trunc);
        if (!stream)
        {
            throw std::runtime_error("Could not write temporary analysis state: " +
                                     temporary_file.string());
        }
        stream << document.dump(2) << '\n';
        if (!stream)
        {
            throw std::runtime_error("Could not write temporary analysis state: " +
                                     temporary_file.string());
        }
    }

    if (!MoveFileExW(temporary_file.c_str(), state_file.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        const std::error_code error(static_cast<int>(GetLastError()), std::system_category());
        std::filesystem::remove(temporary_file);
        throw std::runtime_error("Could not atomically write analysis state: " + error.message());
    }
}

output::AnalysisProgressEvent read_analysis_run_state(const AnalysisRun &run)
{
    std::ifstream stream(run.state_file);
    if (!stream)
    {
        throw std::runtime_error("Could not open analysis state: " + run.state_file.string());
    }

    nlohmann::json document;
    try
    {
        stream >> document;
    }
    catch (const nlohmann::json::parse_error &exception)
    {
        throw std::invalid_argument("Analysis state is not valid JSON: " +
                                    std::string(exception.what()));
    }
    if (!document.is_object() || !document.contains("schemaVersion") ||
        !document.at("schemaVersion").is_number_integer() ||
        document.at("schemaVersion").get<int>() != 1)
    {
        throw std::invalid_argument("Unsupported analysis state schemaVersion.");
    }
    if (!document.contains("state") || !document.at("state").is_string() ||
        !document.contains("message") || !document.at("message").is_string())
    {
        throw std::invalid_argument("Analysis state requires string state and message fields.");
    }

    return output::AnalysisProgressEvent{
        .state = output::parse_analysis_state(document.at("state").get<std::string_view>()),
        .message = document.at("message").get<std::string>(),
    };
}

AnalysisRun prepare_analysis_run(const ProjectFile &project)
{
    if (!std::filesystem::is_directory(project.runs_directory))
    {
        throw std::invalid_argument("Project runs directory does not exist: " +
                                    project.runs_directory.string());
    }
    if (!std::filesystem::is_regular_file(project.input_file))
    {
        throw std::invalid_argument("Project input file does not exist: " +
                                    project.input_file.string());
    }

    std::uint64_t highest_number{};
    for (const auto &entry : std::filesystem::directory_iterator(project.runs_directory))
    {
        if (entry.is_directory())
        {
            highest_number = std::max(highest_number, run_number(entry.path()));
        }
    }
    if (highest_number == std::numeric_limits<std::uint64_t>::max())
    {
        throw std::overflow_error("No additional analysis run number is available.");
    }

    std::uint64_t number = highest_number + 1;
    std::filesystem::path run_directory;
    while (true)
    {
        run_directory = project.runs_directory / run_directory_name(number);
        if (std::filesystem::create_directory(run_directory))
        {
            break;
        }
        if (number == std::numeric_limits<std::uint64_t>::max())
        {
            throw std::overflow_error("No additional analysis run number is available.");
        }
        ++number;
    }

    const auto input_directory = run_directory / "input";
    const auto results_directory = run_directory / "results";
    const auto input_file = input_directory / project.input_file.filename();
    const auto result_file = results_directory / (project.input_file.stem().string() + ".vtu");
    const auto summary_file = results_directory / "analysis-summary.json";
    const auto request_file = run_directory / "analysis-request.json";

    try
    {
        std::filesystem::create_directory(input_directory);
        std::filesystem::create_directory(results_directory);
        std::filesystem::copy_file(project.input_file, input_file);
        input::write_analysis_request(
            request_file,
            input::AnalysisRequest{
                .input_file = std::filesystem::path{"input"} / input_file.filename(),
                .result_file = std::filesystem::path{"results"} / result_file.filename(),
                .summary_file = std::filesystem::path{"results"} / summary_file.filename(),
            });
        write_analysis_run_state(run_directory,
                                 output::AnalysisProgressEvent{output::AnalysisState::preparing,
                                                               "Analysis run prepared."});
    }
    catch (...)
    {
        std::error_code cleanup_error;
        std::filesystem::remove_all(run_directory, cleanup_error);
        throw;
    }

    return AnalysisRun{
        .number = number,
        .run_directory = run_directory,
        .request_file = request_file,
        .input_file = input_file,
        .result_file = result_file,
        .summary_file = summary_file,
        .state_file = run_directory / state_filename,
    };
}
} // namespace finelemethod::project
