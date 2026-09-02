#pragma once

#include <string>
#include <string_view>

namespace finelemethod::gui
{
// Kept independent of wxWidgets so caption semantics can be regression tested.
[[nodiscard]] inline std::string project_caption(const std::string_view project_name,
                                                 const bool recovered_not_saved)
{
    if (project_name.empty())
    {
        return "FinEleMethod";
    }
    std::string caption = "FinEleMethod - ";
    caption += project_name;
    if (recovered_not_saved)
    {
        caption += " [recovered, not saved]";
    }
    return caption;
}
} // namespace finelemethod::gui
