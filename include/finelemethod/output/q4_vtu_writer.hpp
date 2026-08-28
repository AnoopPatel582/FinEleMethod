#pragma once

#include "finelemethod/math/dense_vector.hpp"
#include "finelemethod/model/dof_map.hpp"
#include "finelemethod/model/node_collection.hpp"
#include "finelemethod/model/q4_element_collection.hpp"
#include "finelemethod/postprocessing/q4_plane_stress_results.hpp"

#include <filesystem>
#include <span>
#include <string>

namespace finelemethod::output
{
// Creates an ASCII VTK XML unstructured grid containing Q4 geometry and nodal
// displacement vectors.
[[nodiscard]] std::string create_q4_displacement_vtu(const model::NodeCollection &nodes,
                                                     const model::Q4ElementCollection &elements,
                                                     const model::DofMap &dof_map,
                                                     const math::DenseVector &displacements);

// Adds element CellData by averaging each element's four Gauss-point results.
[[nodiscard]] std::string create_q4_results_vtu(
    const model::NodeCollection &nodes, const model::Q4ElementCollection &elements,
    const model::DofMap &dof_map, const math::DenseVector &displacements,
    std::span<const postprocessing::Q4ElementPlaneStressResults> element_results);

void write_q4_displacement_vtu(const std::filesystem::path &path,
                               const model::NodeCollection &nodes,
                               const model::Q4ElementCollection &elements,
                               const model::DofMap &dof_map,
                               const math::DenseVector &displacements);

void write_q4_results_vtu(
    const std::filesystem::path &path, const model::NodeCollection &nodes,
    const model::Q4ElementCollection &elements, const model::DofMap &dof_map,
    const math::DenseVector &displacements,
    std::span<const postprocessing::Q4ElementPlaneStressResults> element_results);
} // namespace finelemethod::output
