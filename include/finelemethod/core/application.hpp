#pragma once

#include <string>
#include <string_view>

namespace finelemethod
{
[[nodiscard]] std::string_view application_name() noexcept;
// Build diagnostics, not a release version or a unique source revision.
[[nodiscard]] std::string application_build_info();
} // namespace finelemethod
