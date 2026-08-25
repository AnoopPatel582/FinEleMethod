#include "finelemethod/assembly/load_assembly.hpp"

#include <stdexcept>

namespace finelemethod::assembly
{
math::DenseVector assemble_point_load_vector(const model::DofMap &dof_map,
                                             const std::span<const model::PointLoad> point_loads)
{
    if (dof_map.size() == 0)
    {
        throw std::invalid_argument("Point-load assembly requires at least one degree of freedom.");
    }

    math::DenseVector load_vector(dof_map.size());

    for (const model::PointLoad &point_load : point_loads)
    {
        const auto global_index =
            dof_map.global_index(point_load.node_id(), point_load.component());
        load_vector[global_index] += point_load.magnitude();
    }

    return load_vector;
}
} // namespace finelemethod::assembly
