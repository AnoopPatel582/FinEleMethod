#include "finelemethod/core/application.hpp"

#include <climits>
#include <sstream>

#ifndef FINELEMETHOD_BUILD_CONFIGURATION
#define FINELEMETHOD_BUILD_CONFIGURATION "unspecified"
#endif

namespace finelemethod
{
std::string_view application_name() noexcept
{
    return "FinEleMethod command-line solver";
}

std::string application_build_info()
{
    std::ostringstream output;
    output << "FinEleMethod development build\n"
           << "Configuration: " << FINELEMETHOD_BUILD_CONFIGURATION << '\n'
           << "Architecture: " << sizeof(void *) * CHAR_BIT << "-bit\n";
#ifdef _MSC_FULL_VER
    output << "Compiler: MSVC " << _MSC_FULL_VER << '\n';
#else
    output << "Compiler: unspecified\n";
#endif
    return output.str();
}
} // namespace finelemethod
