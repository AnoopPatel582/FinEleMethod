#pragma once

#include "finelemethod/elements/h8_recovery.hpp"
#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/h8_element_collection.hpp"
#include "finelemethod/model/material_collection.hpp"
#include "finelemethod/model/node_collection.hpp"

#include <vector>

namespace finelemethod::postprocessing
{
struct H8ElementResults
{
    model::ElementId element_id;
    elements::H8GaussResults gauss_points;
};

// Recovers Gauss-point results for every H8 element from the solved global
// displacement vector. Element results preserve collection insertion order.
[[nodiscard]] std::vector<H8ElementResults> recover_h8_model_results(
    const model::H8ElementCollection &elements, const model::NodeCollection &nodes,
    const model::MaterialCollection &materials, const model::DofMap &dof_map,
    const math::DenseVector &global_displacements);
} // namespace finelemethod::postprocessing
