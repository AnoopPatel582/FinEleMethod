#include "finelemethod/cli/command_line.hpp"

#include "finelemethod/core/application.hpp"
#include "finelemethod/examples/q4_tension_example.hpp"

#include <filesystem>
#include <ostream>
#include <stdexcept>
#include <string>

namespace finelemethod::cli
{
namespace
{
void write_help(std::ostream &stream)
{
    stream << application_name() << '\n'
           << "Usage:\n"
           << "  FinEleMethod --help\n"
           << "  FinEleMethod --example q4-tension --output <file.vtu>\n\n"
           << "Options:\n"
           << "  -h, --help  Show this help message.\n"
           << "  --example q4-tension  Run the built-in Q4 uniaxial-tension example.\n"
           << "  --output <file.vtu>   Write the example results to an ASCII VTU file.\n";
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

    if (arguments.size() == 4 && arguments[0] == "--example" && arguments[1] == "q4-tension" &&
        arguments[2] == "--output")
    {
        const std::filesystem::path output_path{std::string(arguments[3])};
        try
        {
            examples::write_q4_tension_example(output_path);
            output << "Completed Q4 tension example.\n"
                   << "VTU result: " << output_path.string() << '\n';
            return ExitCode::Success;
        }
        catch (const std::invalid_argument &exception)
        {
            error << "Model validation error: " << exception.what() << '\n';
            return ExitCode::ModelValidationError;
        }
        catch (const std::runtime_error &exception)
        {
            error << "Result-writing error: " << exception.what() << '\n';
            return ExitCode::ResultWritingError;
        }
        catch (const std::exception &exception)
        {
            error << "Unexpected internal error: " << exception.what() << '\n';
            return ExitCode::UnexpectedInternalError;
        }
    }

    error << "Error: unknown command-line argument." << '\n';
    return ExitCode::UsageError;
}
} // namespace finelemethod::cli
