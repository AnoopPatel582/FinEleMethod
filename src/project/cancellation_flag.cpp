#include "finelemethod/project/cancellation_flag.hpp"

#include <fstream>
#include <stdexcept>

namespace finelemethod::project
{
namespace
{
constexpr const char *cancellation_filename = "cancellation-requested.flag";
}

std::filesystem::path cancellation_flag_path(const std::filesystem::path &run_directory)
{
    return run_directory / cancellation_filename;
}

void request_analysis_cancellation(const std::filesystem::path &run_directory)
{
    if (!std::filesystem::is_directory(run_directory))
    {
        throw std::invalid_argument("Analysis run directory does not exist: " +
                                    run_directory.string());
    }

    const auto flag = cancellation_flag_path(run_directory);
    std::ofstream stream(flag, std::ios::trunc);
    if (!stream)
    {
        throw std::runtime_error("Could not create analysis cancellation flag: " + flag.string());
    }
    stream << "Cancellation requested.\n";
    if (!stream)
    {
        throw std::runtime_error("Could not write analysis cancellation flag: " + flag.string());
    }
}

bool is_analysis_cancellation_requested(const std::filesystem::path &run_directory)
{
    return std::filesystem::is_regular_file(cancellation_flag_path(run_directory));
}
} // namespace finelemethod::project
