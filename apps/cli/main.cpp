#include "finelemethod/cli/command_line.hpp"
#include "finelemethod/core/exit_code.hpp"

#include <cstdio>
#include <exception>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

int main(const int argument_count, char *argument_values[])
{
    try
    {
        std::vector<std::string_view> arguments;
        arguments.reserve(static_cast<std::size_t>(argument_count - 1));

        for (int index = 1; index < argument_count; ++index)
        {
            arguments.emplace_back(argument_values[index]);
        }

        const std::span<const std::string_view> argument_view(arguments);
        const auto exit_code = finelemethod::cli::run(argument_view, std::cout, std::cerr);
        return static_cast<int>(exit_code);
    }
    catch (const std::exception &exception)
    {
        std::fprintf(stderr, "Unexpected internal error: %s\n", exception.what());
    }
    catch (...)
    {
        std::fprintf(stderr, "Unexpected unknown internal error.\n");
    }
    return static_cast<int>(finelemethod::ExitCode::UnexpectedInternalError);
}
