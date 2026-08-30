#pragma once

#include "finelemethod/elements/q4_plane_strain_recovery.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/q4_element_collection.hpp"

#include <vector>

namespace finelemethod::postprocessing
{
struct Q4ElementPlaneStrainResults
{
    model::ElementId element_id;
    elements::Q4PlaneStrainGaussResults gauss_points;
};

// Recovers Gauss-point results for every Q4 element from the solved global
// displacement vector. Element results preserve collection insertion order.
[[nodiscard]] std::vector<Q4ElementPlaneStrainResults> recover_q4_plane_strain_model_results(
    const model::Q4ElementCollection &elements, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map,
    const math::DenseVector &global_displacements);
} // namespace finelemethod::postprocessing
