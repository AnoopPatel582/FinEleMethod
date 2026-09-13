#pragma once

#include "finelemethod/solver/abaqus_imported_analysis.hpp"

#include <filesystem>
#include <string>

namespace finelemethod::output
{
[[nodiscard]] std::string create_abaqus_imported_vtu(
    const solver::AbaqusImportedAnalysisSolution &solution);

void write_abaqus_imported_vtu(const std::filesystem::path &path,
                               const solver::AbaqusImportedAnalysisSolution &solution);
} // namespace finelemethod::output
