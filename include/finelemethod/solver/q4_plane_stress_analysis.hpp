#pragma once

#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/point_load.hpp"
#include "finelemethod/model/q4_edge_pressure_load.hpp"
#include "finelemethod/model/q4_element_collection.hpp"
#include "finelemethod/postprocessing/q4_plane_stress_results.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"
#include "finelemethod/solver/sparse_static_solver.hpp"

#include <span>
#include <vector>

namespace finelemethod::solver
{
struct Q4PlaneStressAnalysisResult
{
    math::DenseVector displacements;
    math::DenseVector reactions;
    std::vector<postprocessing::Q4ElementPlaneStressResults> element_results;
};

// Runs a complete sparse linear-static Q4 plane-stress analysis.
[[nodiscard]] Q4PlaneStressAnalysisResult solve_q4_plane_stress_model(
    const model::Q4ElementCollection &elements, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map,
    std::span<const model::PointLoad> point_loads,
    std::span<const model::Q4EdgePressureLoad> pressure_loads,
    std::span<const PrescribedDisplacement> prescribed_displacements);
} // namespace finelemethod::solver
