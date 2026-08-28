#include "finelemethod/output/q4_vtu_writer.hpp"

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace finelemethod::output
{
std::string create_q4_displacement_vtu(const model::NodeCollection &nodes,
                                       const model::Q4ElementCollection &elements,
                                       const model::DofMap &dof_map,
                                       const math::DenseVector &displacements)
{
    if (nodes.empty())
    {
        throw std::invalid_argument("VTU output requires at least one node.");
    }
    if (dof_map.spatial_dimension() != model::SpatialDimension::two_dimensional)
    {
        throw std::invalid_argument("Q4 VTU output requires a two-dimensional DOF map.");
    }
    if (displacements.size() != dof_map.size())
    {
        throw std::invalid_argument(
            "Displacement vector size must match the number of degrees of freedom.");
    }

    std::unordered_map<model::NodeId, std::size_t> point_indices;
    point_indices.reserve(nodes.size());
    for (std::size_t point_index = 0; point_index < nodes.size(); ++point_index)
    {
        point_indices.emplace(nodes.nodes()[point_index].id(), point_index);
    }

    std::ostringstream output;
    output << std::setprecision(17) << std::scientific;
    output << "<?xml version=\"1.0\"?>\n"
           << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">\n"
           << "  <UnstructuredGrid>\n"
           << "    <Piece NumberOfPoints=\"" << nodes.size() << "\" NumberOfCells=\""
           << elements.size() << "\">\n"
           << "      <PointData Vectors=\"Displacement\">\n"
           << "        <DataArray type=\"Float64\" Name=\"Displacement\" NumberOfComponents=\"3\" "
              "format=\"ascii\">\n";

    for (const model::Node &node : nodes.nodes())
    {
        output << "          "
               << displacements[dof_map.global_index(node.id(), model::DisplacementComponent::x)]
               << ' '
               << displacements[dof_map.global_index(node.id(), model::DisplacementComponent::y)]
               << " 0.00000000000000000e+00\n";
    }

    output << "        </DataArray>\n"
           << "      </PointData>\n"
           << "      <Points>\n"
           << "        <DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">\n";

    for (const model::Node &node : nodes.nodes())
    {
        output << "          " << node.x() << ' ' << node.y() << ' ' << node.z() << '\n';
    }

    output << "        </DataArray>\n"
           << "      </Points>\n"
           << "      <Cells>\n"
           << "        <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n";

    for (const model::Q4Element &element : elements.elements())
    {
        output << "          ";
        for (std::size_t node_index = 0; node_index < element.node_ids().size(); ++node_index)
        {
            if (node_index != 0)
            {
                output << ' ';
            }
            output << point_indices.at(element.node_ids()[node_index]);
        }
        output << '\n';
    }

    output << "        </DataArray>\n"
           << "        <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n"
           << "          ";
    for (std::size_t cell_index = 0; cell_index < elements.size(); ++cell_index)
    {
        if (cell_index != 0)
        {
            output << ' ';
        }
        output << 4 * (cell_index + 1);
    }
    output << "\n        </DataArray>\n"
           << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n"
           << "          ";
    for (std::size_t cell_index = 0; cell_index < elements.size(); ++cell_index)
    {
        if (cell_index != 0)
        {
            output << ' ';
        }
        output << 9;
    }
    output << "\n        </DataArray>\n"
           << "      </Cells>\n"
           << "    </Piece>\n"
           << "  </UnstructuredGrid>\n"
           << "</VTKFile>\n";

    return output.str();
}

void write_q4_displacement_vtu(const std::filesystem::path &path,
                               const model::NodeCollection &nodes,
                               const model::Q4ElementCollection &elements,
                               const model::DofMap &dof_map, const math::DenseVector &displacements)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        throw std::runtime_error("Unable to open VTU output file: " + path.string());
    }

    file << create_q4_displacement_vtu(nodes, elements, dof_map, displacements);
    if (!file)
    {
        throw std::runtime_error("Unable to write VTU output file: " + path.string());
    }
}
} // namespace finelemethod::output
