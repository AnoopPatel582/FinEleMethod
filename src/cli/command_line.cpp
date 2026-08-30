#include "finelemethod/cli/command_line.hpp"

#include "finelemethod/core/application.hpp"
#include "finelemethod/examples/q4_tension_example.hpp"
#include "finelemethod/input/abaqus_input_file.hpp"
#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/output/q4_analysis_vtu.hpp"
#include "finelemethod/solver/abaqus_q4_analysis.hpp"

#include <filesystem>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <variant>

namespace finelemethod::cli
{
namespace
{
void write_help(std::ostream &stream)
{
    stream << application_name() << '\n'
           << "Usage:\n"
           << "  FinEleMethod --help\n"
           << "  FinEleMethod --input <model.inp> --output <result.vtu>\n"
           << "  FinEleMethod --example q4-tension --output <file.vtu>\n\n"
           << "Options:\n"
           << "  -h, --help  Show this help message.\n"
           << "  --input <model.inp>   Read and solve an ABAQUS Q4 CPS4 or CPE4 model.\n"
           << "  --example q4-tension  Run the built-in Q4 uniaxial-tension example.\n"
           << "  --output <file.vtu>   Write analysis results to an ASCII VTU file.\n";
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

    if (arguments.size() == 4 && arguments[0] == "--input" && arguments[2] == "--output")
    {
        const std::filesystem::path input_path{std::string(arguments[1])};
        const std::filesystem::path output_path{std::string(arguments[3])};
        std::string input_text;
        try
        {
            input_text = input::read_abaqus_input_file(input_path);
        }
        catch (const std::exception &exception)
        {
            error << "Input-file error: " << exception.what() << '\n';
            return ExitCode::InputParsingError;
        }

        std::optional<solver::AbaqusQ4Solution> solution;
        try
        {
            solution.emplace(solver::analyze_abaqus_q4(input_text));
        }
        catch (const input::AbaqusParseError &exception)
        {
            error << "Input-parsing error: " << exception.what() << '\n';
            return ExitCode::InputParsingError;
        }
        catch (const std::invalid_argument &exception)
        {
            error << "Model validation error: " << exception.what() << '\n';
            return ExitCode::ModelValidationError;
        }
        catch (const std::runtime_error &exception)
        {
            error << "Numerical solution error: " << exception.what() << '\n';
            return ExitCode::NumericalSolutionError;
        }
        catch (const std::exception &exception)
        {
            error << "Unexpected internal error: " << exception.what() << '\n';
            return ExitCode::UnexpectedInternalError;
        }

        try
        {
            const model::DofMap dof_map(solution->model.nodes,
                                        model::SpatialDimension::two_dimensional);
            std::visit(
                [&](const auto &result) {
                    output::write_q4_analysis_vtu(output_path, solution->model.nodes,
                                                  solution->model.elements, dof_map, result);
                },
                solution->result);
            const std::string_view formulation =
                solution->model.analysis_type == input::Q4AnalysisType::plane_stress
                    ? "plane-stress"
                    : "plane-strain";
            output << "Completed ABAQUS Q4 " << formulation << " analysis.\n"
                   << "Input model: " << input_path.string() << '\n'
                   << "VTU result: " << output_path.string() << '\n';
            return ExitCode::Success;
        }
        catch (const std::exception &exception)
        {
            error << "Result-writing error: " << exception.what() << '\n';
            return ExitCode::ResultWritingError;
        }
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
