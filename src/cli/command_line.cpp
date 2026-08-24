#include "finelemethod/cli/command_line.hpp"

#include "finelemethod/core/application.hpp"

#include <ostream>

namespace finelemethod::cli
{
namespace
{
void write_help(std::ostream &stream)
{
    stream << application_name() << '\n'
           << "Usage:\n"
           << "  FinEleMethod --help\n\n"
           << "Options:\n"
           << "  -h, --help  Show this help message.\n\n"
           << "Analysis execution is not available yet.\n";
}
} // namespace

ExitCode run(const std::span<const std::string_view> arguments, std::ostream &output,
             std::ostream &error)
{
    if (arguments.empty())
    {
        error << "Error: an analysis request is required.\n\n";
        write_help(error);
        return ExitCode::UsageError;
    }

    if (arguments.size() == 1 && (arguments.front() == "--help" || arguments.front() == "-h"))
    {
        write_help(output);
        return ExitCode::Success;
    }

    error << "Error: unknown command-line argument." << '\n';
    return ExitCode::UsageError;
}
} // namespace finelemethod::cli
