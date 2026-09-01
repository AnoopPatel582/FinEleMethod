#include "finelemethod/project/analysis_run.hpp"

#include "finelemethod/input/analysis_request.hpp"

#include <algorithm>
#include <charconv>
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
} // namespace

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
    };
}
} // namespace finelemethod::project
