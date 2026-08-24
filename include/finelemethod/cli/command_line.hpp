#pragma once

#include "finelemethod/core/exit_code.hpp"

#include <iosfwd>
#include <span>
#include <string_view>

namespace finelemethod::cli
{
[[nodiscard]] ExitCode run(std::span<const std::string_view> arguments, std::ostream &output,
                           std::ostream &error);
} // namespace finelemethod::cli
