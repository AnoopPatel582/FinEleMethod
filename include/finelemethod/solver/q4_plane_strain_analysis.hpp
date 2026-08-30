#pragma once

#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/point_load.hpp"
#include "finelemethod/model/q4_edge_pressure_load.hpp"
#include "finelemethod/model/q4_element_collection.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"
#include "finelemethod/solver/dense_static_solver.hpp"

#include <span>

namespace finelemethod::solver
{
struct Q4PlaneStrainAnalysisResult
{
    math::DenseVector displacements;
    math::DenseVector reactions;
};

// Runs a dense linear-static Q4 plane-strain analysis without result recovery.
[[nodiscard]] Q4PlaneStrainAnalysisResult solve_q4_plane_strain_model(
    const model::Q4ElementCollection &elements, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map,
    std::span<const model::PointLoad> point_loads,
    std::span<const model::Q4EdgePressureLoad> pressure_loads,
    std::span<const PrescribedDisplacement> prescribed_displacements);
} // namespace finelemethod::solver
