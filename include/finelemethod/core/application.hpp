#pragma once

#include <string>
#include <string_view>

namespace finelemethod
{
[[nodiscard]] std::string_view application_name() noexcept;
// Version and build diagnostics; this is not a unique source revision.
[[nodiscard]] std::string application_build_info();
} // namespace finelemethod
