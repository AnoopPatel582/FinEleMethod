#pragma once

#include "finelemethod/model/node.hpp"

#include <string>
#include <variant>

namespace finelemethod::input
{
// ABAQUS nodal data can target either one numeric node ID or a named node set.
using AbaqusNodeTarget = std::variant<model::NodeId, std::string>;
} // namespace finelemethod::input
