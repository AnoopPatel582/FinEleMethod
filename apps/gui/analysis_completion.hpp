#pragma once

#include <exception>
#include <optional>
#include <string>

namespace finelemethod::gui
{
// Restore interaction before optional history I/O. A history warning must not
// short-circuit validation of the solver outcome. This sequencing boundary is
// independent of wxWidgets so tests can inject disk failures.
template <typename RestoreControls, typename RefreshHistory>
[[nodiscard]] std::optional<std::string> restore_after_analysis(RestoreControls restore_controls,
                                                                RefreshHistory refresh_history)
{
    restore_controls();
    try
    {
        refresh_history();
        return std::nullopt;
    }
    catch (const std::exception &exception)
    {
        return exception.what();
    }
}
} // namespace finelemethod::gui
