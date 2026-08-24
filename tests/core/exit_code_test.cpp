#include "finelemethod/core/exit_code.hpp"

#include <gtest/gtest.h>

TEST(ExitCode, PreservesProtocolValues)
{
    using finelemethod::ExitCode;

    EXPECT_EQ(static_cast<int>(ExitCode::Success), 0);
    EXPECT_EQ(static_cast<int>(ExitCode::UsageError), 1);
    EXPECT_EQ(static_cast<int>(ExitCode::InputParsingError), 2);
    EXPECT_EQ(static_cast<int>(ExitCode::ModelValidationError), 3);
    EXPECT_EQ(static_cast<int>(ExitCode::NumericalSolutionError), 4);
    EXPECT_EQ(static_cast<int>(ExitCode::ResultWritingError), 5);
    EXPECT_EQ(static_cast<int>(ExitCode::Cancelled), 6);
    EXPECT_EQ(static_cast<int>(ExitCode::UnexpectedInternalError), 10);
}
