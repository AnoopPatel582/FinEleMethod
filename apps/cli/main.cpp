#include "finelemethod/core/application.hpp"
#include "finelemethod/core/exit_code.hpp"

#include <iostream>

int main()
{
    std::cout << finelemethod::application_name() << '\n';
    return static_cast<int>(finelemethod::ExitCode::Success);
}
