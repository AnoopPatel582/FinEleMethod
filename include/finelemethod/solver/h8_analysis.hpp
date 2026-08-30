#pragma once

#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/h8_element_collection.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/point_load.hpp"
#include "finelemethod/solver/boundary_conditions.hpp"
#include "finelemethod/solver/dense_static_solver.hpp"

#include <span>

namespace finelemethod::solver
{
struct H8AnalysisResult
{
    math::DenseVector displacements;
    math::DenseVector reactions;
};

// Runs a dense linear-static H8 analysis with nodal point loads.
[[nodiscard]] H8AnalysisResult solve_h8_model(
    const model::H8ElementCollection &elements, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map,
    std::span<const model::PointLoad> point_loads,
    std::span<const PrescribedDisplacement> prescribed_displacements);
} // namespace finelemethod::solver
