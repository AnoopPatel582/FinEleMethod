#pragma once

#include <stdexcept>

namespace finelemethod::input
{
class AbaqusParseError : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
} // namespace finelemethod::input
