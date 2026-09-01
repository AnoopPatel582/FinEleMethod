#include "finelemethod/cli/command_line.hpp"

#include "finelemethod/core/application.hpp"
#include "finelemethod/examples/h8_compression_example.hpp"
#include "finelemethod/examples/q4_tension_example.hpp"
#include "finelemethod/input/abaqus_element_parser.hpp"
#include "finelemethod/input/abaqus_input_file.hpp"
#include "finelemethod/input/abaqus_parse_error.hpp"
#include "finelemethod/input/analysis_request.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/output/analysis_progress.hpp"
#include "finelemethod/output/analysis_summary.hpp"
#include "finelemethod/output/h8_analysis_vtu.hpp"
#include "finelemethod/output/q4_analysis_vtu.hpp"
#include "finelemethod/project/cancellation_flag.hpp"
#include "finelemethod/solver/abaqus_h8_analysis.hpp"
#include "finelemethod/solver/abaqus_q4_analysis.hpp"

#include <array>
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
           << "  FinEleMethod --request <analysis-request.json>\n"
           << "  FinEleMethod --input <model.inp> --output <result.vtu> "
              "[--summary <summary.json>] [--json-progress]\n"
           << "  FinEleMethod --example q4-tension --output <file.vtu>\n"
           << "  FinEleMethod --example h8-compression --output <file.vtu>\n\n"
           << "Options:\n"
           << "  -h, --help  Show this help message.\n"
           << "  --request <file.json>  Run a versioned analysis request.\n"
           << "  --input <model.inp>   Read and solve an ABAQUS CPS4, CPE4, or C3D8 model.\n"
           << "  --example q4-tension  Run the built-in Q4 uniaxial-tension example.\n"
           << "  --example h8-compression  Run the built-in H8 block-compression example.\n"
           << "  --output <file.vtu>   Write analysis results to an ASCII VTU file.\n";
    stream << "  --summary <file.json>  Write a versioned analysis summary.\n"
           << "  --json-progress       Write versioned JSON Lines progress to stdout.\n";
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

    if (arguments.size() == 2 && arguments[0] == "--request")
    {
        const std::filesystem::path request_path{std::string(arguments[1])};
        try
        {
            const input::AnalysisRequest request = input::read_analysis_request(request_path);
            const std::filesystem::path request_directory = request_path.parent_path();
            const std::array<std::string, 9> owned_arguments{
                "--input",
                (request_directory / request.input_file).lexically_normal().string(),
                "--output",
                (request_directory / request.result_file).lexically_normal().string(),
                "--summary",
                (request_directory / request.summary_file).lexically_normal().string(),
                "--json-progress",
                "--cancellation-directory",
                request_directory.lexically_normal().string(),
            };
            const std::array<std::string_view, 9> request_arguments{
                owned_arguments[0], owned_arguments[1], owned_arguments[2],
                owned_arguments[3], owned_arguments[4], owned_arguments[5],
                owned_arguments[6], owned_arguments[7], owned_arguments[8],
            };
            return run(request_arguments, output, error);
        }
        catch (const std::exception &exception)
        {
            output::write_analysis_progress_json_line(
                output, output::AnalysisProgressEvent{output::AnalysisState::preparing,
                                                      "Reading analysis request."});
            output::write_analysis_progress_json_line(
                output,
                output::AnalysisProgressEvent{output::AnalysisState::failed, exception.what()});
            error << "Analysis-request error: " << exception.what() << '\n';
            return ExitCode::InputParsingError;
        }
    }

    const bool has_summary = arguments.size() >= 6 && arguments[4] == "--summary";
    const bool has_json_progress = (arguments.size() == 5 && arguments[4] == "--json-progress") ||
                                   (arguments.size() == 7 && arguments[6] == "--json-progress") ||
                                   (arguments.size() == 9 && arguments[6] == "--json-progress");
    const bool has_cancellation_directory =
        arguments.size() == 9 && arguments[7] == "--cancellation-directory";
    const bool is_input_analysis =
        arguments.size() >= 4 && arguments.size() <= 9 && arguments[0] == "--input" &&
        arguments[2] == "--output" &&
        ((arguments.size() == 4) || (arguments.size() == 5 && has_json_progress) ||
         (arguments.size() == 6 && has_summary) ||
         (arguments.size() == 7 && has_summary && has_json_progress) ||
         (arguments.size() == 9 && has_summary && has_json_progress && has_cancellation_directory));
    if (is_input_analysis)
    {
        const std::filesystem::path input_path{std::string(arguments[1])};
        const std::filesystem::path output_path{std::string(arguments[3])};
        const std::optional<std::filesystem::path> summary_path =
            has_summary ? std::optional<std::filesystem::path>{std::string(arguments[5])}
                        : std::nullopt;
        const std::optional<std::filesystem::path> cancellation_directory =
            has_cancellation_directory
                ? std::optional<std::filesystem::path>{std::string(arguments[8])}
                : std::nullopt;
        const auto write_progress = [&](const output::AnalysisState state,
                                        const std::string_view message) {
            if (has_json_progress)
            {
                output::write_analysis_progress_json_line(
                    output, output::AnalysisProgressEvent{state, std::string(message)});
            }
        };
        const auto cancellation_requested = [&] {
            if (!cancellation_directory ||
                !project::is_analysis_cancellation_requested(*cancellation_directory))
            {
                return false;
            }
            write_progress(output::AnalysisState::cancelled, "Analysis cancelled by user.");
            return true;
        };

        write_progress(output::AnalysisState::preparing, "Reading input model.");
        if (cancellation_requested())
        {
            return ExitCode::Cancelled;
        }
        std::string input_text;
        try
        {
            input_text = input::read_abaqus_input_file(input_path);
        }
        catch (const std::exception &exception)
        {
            write_progress(output::AnalysisState::failed, exception.what());
            error << "Input-file error: " << exception.what() << '\n';
            return ExitCode::InputParsingError;
        }

        try
        {
            if (cancellation_requested())
            {
                return ExitCode::Cancelled;
            }
            write_progress(output::AnalysisState::executing, "Solving finite element model.");
            const input::AbaqusElementFamily family =
                input::detect_abaqus_element_family(input_text);
            if (family == input::AbaqusElementFamily::h8)
            {
                auto solution = solver::analyze_abaqus_h8(input_text);
                const model::DofMap dof_map(solution.model.nodes,
                                            model::SpatialDimension::three_dimensional);
                if (cancellation_requested())
                {
                    return ExitCode::Cancelled;
                }
                try
                {
                    write_progress(output::AnalysisState::writing_results,
                                   "Writing analysis results.");
                    output::write_h8_analysis_vtu(output_path, solution.model.nodes,
                                                  solution.model.elements, dof_map,
                                                  solution.result);
                    if (summary_path)
                    {
                        output::write_analysis_summary(
                            *summary_path,
                            output::AnalysisSummary{
                                .analysis_type = "h8-three-dimensional",
                                .input_path = input_path,
                                .result_path = output_path,
                                .node_count = solution.model.nodes.size(),
                                .element_count = solution.model.elements.size(),
                                .solver_iterations = solution.result.solver_iterations,
                                .residual_norm = solution.result.residual_norm,
                            });
                    }
                }
                catch (const std::exception &exception)
                {
                    write_progress(output::AnalysisState::failed, exception.what());
                    error << "Result-writing error: " << exception.what() << '\n';
                    return ExitCode::ResultWritingError;
                }
                write_progress(output::AnalysisState::completed, "Analysis completed.");
                if (!has_json_progress)
                {
                    output << "Completed ABAQUS H8 analysis.\n"
                           << "Input model: " << input_path.string() << '\n'
                           << "VTU result: " << output_path.string() << '\n';
                    if (summary_path)
                    {
                        output << "Analysis summary: " << summary_path->string() << '\n';
                    }
                }
                return ExitCode::Success;
            }

            auto solution = solver::analyze_abaqus_q4(input_text);
            const model::DofMap dof_map(solution.model.nodes,
                                        model::SpatialDimension::two_dimensional);
            if (cancellation_requested())
            {
                return ExitCode::Cancelled;
            }
            try
            {
                write_progress(output::AnalysisState::writing_results, "Writing analysis results.");
                std::visit(
                    [&](const auto &result) {
                        output::write_q4_analysis_vtu(output_path, solution.model.nodes,
                                                      solution.model.elements, dof_map, result);
                    },
                    solution.result);
                if (summary_path)
                {
                    const std::string analysis_type =
                        solution.model.analysis_type == input::Q4AnalysisType::plane_stress
                            ? "q4-plane-stress"
                            : "q4-plane-strain";
                    std::visit(
                        [&](const auto &result) {
                            output::write_analysis_summary(
                                *summary_path, output::AnalysisSummary{
                                                   .analysis_type = analysis_type,
                                                   .input_path = input_path,
                                                   .result_path = output_path,
                                                   .node_count = solution.model.nodes.size(),
                                                   .element_count = solution.model.elements.size(),
                                                   .solver_iterations = result.solver_iterations,
                                                   .residual_norm = result.residual_norm,
                                               });
                        },
                        solution.result);
                }
            }
            catch (const std::exception &exception)
            {
                write_progress(output::AnalysisState::failed, exception.what());
                error << "Result-writing error: " << exception.what() << '\n';
                return ExitCode::ResultWritingError;
            }
            const std::string_view formulation =
                solution.model.analysis_type == input::Q4AnalysisType::plane_stress
                    ? "plane-stress"
                    : "plane-strain";
            write_progress(output::AnalysisState::completed, "Analysis completed.");
            if (!has_json_progress)
            {
                output << "Completed ABAQUS Q4 " << formulation << " analysis.\n"
                       << "Input model: " << input_path.string() << '\n'
                       << "VTU result: " << output_path.string() << '\n';
                if (summary_path)
                {
                    output << "Analysis summary: " << summary_path->string() << '\n';
                }
            }
            return ExitCode::Success;
        }
        catch (const input::AbaqusParseError &exception)
        {
            write_progress(output::AnalysisState::failed, exception.what());
            error << "Input-parsing error: " << exception.what() << '\n';
            return ExitCode::InputParsingError;
        }
        catch (const std::invalid_argument &exception)
        {
            write_progress(output::AnalysisState::failed, exception.what());
            error << "Model validation error: " << exception.what() << '\n';
            return ExitCode::ModelValidationError;
        }
        catch (const std::runtime_error &exception)
        {
            write_progress(output::AnalysisState::failed, exception.what());
            error << "Numerical solution error: " << exception.what() << '\n';
            return ExitCode::NumericalSolutionError;
        }
        catch (const std::exception &exception)
        {
            write_progress(output::AnalysisState::failed, exception.what());
            error << "Unexpected internal error: " << exception.what() << '\n';
            return ExitCode::UnexpectedInternalError;
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

    if (arguments.size() == 4 && arguments[0] == "--example" && arguments[1] == "h8-compression" &&
        arguments[2] == "--output")
    {
        const std::filesystem::path output_path{std::string(arguments[3])};
        try
        {
            examples::write_h8_compression_example(output_path);
            output << "Completed H8 compression example.\n"
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
