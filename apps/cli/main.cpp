#include "finelemethod/cli/command_line.hpp"

#include <iostream>
#include <string_view>
#include <vector>

int main(const int argument_count, char *argument_values[])
{
    std::vector<std::string_view> arguments;
    arguments.reserve(static_cast<std::size_t>(argument_count - 1));

    for (int index = 1; index < argument_count; ++index)
    {
        arguments.emplace_back(argument_values[index]);
    }

    const auto exit_code = finelemethod::cli::run(arguments, std::cout, std::cerr);
    return static_cast<int>(exit_code);
}
