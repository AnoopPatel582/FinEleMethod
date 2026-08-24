#pragma once

namespace finelemethod
{
enum class ExitCode : int
{
    Success = 0,
    UsageError = 1,
    InputParsingError = 2,
    ModelValidationError = 3,
    NumericalSolutionError = 4,
    ResultWritingError = 5,
    Cancelled = 6,
    UnexpectedInternalError = 10
};
} // namespace finelemethod
