#pragma once

#include "finelemethod/input/abaqus_cae_importer.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/solver/conjugate_gradient.hpp"

#include <array>
#include <cstddef>
#include <string_view>
#include <vector>

namespace finelemethod::solver
{
struct AbaqusImportedElementResult
{
    model::ElementId element_id;
    // Engineering-strain order: xx, yy, zz, xy, yz, zx.
    std::array<double, 6> strain{};
    // Stress order: xx, yy, zz, xy, yz, zx.
    std::array<double, 6> stress{};
    double von_mises{};
    std::array<double, 3> principal_stresses{};
};

struct AbaqusImportedAnalysisSolution
{
    input::AbaqusImportedModel model;
    math::DenseVector displacements;
    math::DenseVector reactions;
    std::vector<AbaqusImportedElementResult> element_results;
    std::size_t solver_iterations{};
    double residual_norm{};
};

// Runs the canonical imported-model path for CPS3, CPS4R, C3D4, or C3D8R.
[[nodiscard]] AbaqusImportedAnalysisSolution analyze_abaqus_imported_model(
    std::string_view input_text, const ConjugateGradientOptions &solver_options = {});
} // namespace finelemethod::solver
