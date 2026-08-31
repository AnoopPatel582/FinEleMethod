#include "finelemethod/output/h8_analysis_vtu.hpp"

#include <array>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>

namespace finelemethod::output
{
namespace
{
template <typename Member>
void write_component_averages(std::ostringstream &output, const std::string_view name,
                              const std::vector<postprocessing::H8ElementResults> &element_results,
                              Member member)
{
    using ValueArray =
        std::remove_cvref_t<decltype(element_results.front().gauss_points.front().*member)>;
    constexpr std::size_t component_count = std::tuple_size_v<ValueArray>;
    output << "        <DataArray type=\"Float64\" Name=\"" << name << "\" NumberOfComponents=\""
           << component_count << "\" format=\"ascii\">\n";
    for (const auto &element_result : element_results)
    {
        std::array<double, component_count> average{};
        for (const auto &point : element_result.gauss_points)
        {
            const auto &values = point.*member;
            for (std::size_t component = 0; component < average.size(); ++component)
            {
                average[component] += values[component] / 8.0;
            }
        }
        output << "          ";
        for (std::size_t component = 0; component < average.size(); ++component)
        {
            if (component != 0)
            {
                output << ' ';
            }
            output << average[component];
        }
        output << '\n';
    }
    output << "        </DataArray>\n";
}
} // namespace

std::string create_h8_analysis_vtu(const model::NodeCollection &nodes,
                                   const model::H8ElementCollection &elements,
                                   const model::DofMap &dof_map,
                                   const solver::H8AnalysisResult &analysis_result)
{
    if (nodes.empty())
    {
        throw std::invalid_argument("H8 VTU output requires at least one node.");
    }
    if (dof_map.spatial_dimension() != model::SpatialDimension::three_dimensional)
    {
        throw std::invalid_argument("H8 VTU output requires a three-dimensional DOF map.");
    }
    if (analysis_result.displacements.size() != dof_map.size() ||
        analysis_result.reactions.size() != dof_map.size())
    {
        throw std::invalid_argument(
            "H8 VTU displacement and reaction sizes must match the DOF map.");
    }
    if (analysis_result.element_results.size() != elements.size())
    {
        throw std::invalid_argument("H8 VTU element-result count must match the element count.");
    }
    for (std::size_t index = 0; index < elements.size(); ++index)
    {
        if (analysis_result.element_results[index].element_id != elements.elements()[index].id())
        {
            throw std::invalid_argument(
                "H8 VTU element results must follow the element collection order.");
        }
    }

    std::unordered_map<model::NodeId, std::size_t> point_indices;
    point_indices.reserve(nodes.size());
    for (std::size_t index = 0; index < nodes.size(); ++index)
    {
        point_indices.emplace(nodes.nodes()[index].id(), index);
    }

    std::ostringstream output;
    output << std::setprecision(17) << std::scientific;
    output << "<?xml version=\"1.0\"?>\n"
           << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" "
              "byte_order=\"LittleEndian\">\n"
           << "  <UnstructuredGrid>\n"
           << "    <Piece NumberOfPoints=\"" << nodes.size() << "\" NumberOfCells=\""
           << elements.size() << "\">\n"
           << "      <CellData Scalars=\"VonMises\">\n";

    write_component_averages(output, "Strain", analysis_result.element_results,
                             &elements::H8PointResult::strain);
    write_component_averages(output, "Stress", analysis_result.element_results,
                             &elements::H8PointResult::stress);
    output << "        <DataArray type=\"Float64\" Name=\"VonMises\" format=\"ascii\">\n";
    for (const auto &element_result : analysis_result.element_results)
    {
        double average = 0.0;
        for (const auto &point : element_result.gauss_points)
        {
            average += point.von_mises / 8.0;
        }
        output << "          " << average << '\n';
    }
    output << "        </DataArray>\n";
    write_component_averages(output, "PrincipalStress", analysis_result.element_results,
                             &elements::H8PointResult::principal_stresses);

    output << "      </CellData>\n"
           << "      <PointData Vectors=\"Displacement\">\n"
           << "        <DataArray type=\"Float64\" Name=\"Displacement\" "
              "NumberOfComponents=\"3\" format=\"ascii\">\n";
    for (const model::Node &node : nodes.nodes())
    {
        output
            << "          "
            << analysis_result
                   .displacements[dof_map.global_index(node.id(), model::DisplacementComponent::x)]
            << ' '
            << analysis_result
                   .displacements[dof_map.global_index(node.id(), model::DisplacementComponent::y)]
            << ' '
            << analysis_result
                   .displacements[dof_map.global_index(node.id(), model::DisplacementComponent::z)]
            << '\n';
    }
    output << "        </DataArray>\n"
           << "        <DataArray type=\"Float64\" Name=\"ReactionForce\" "
              "NumberOfComponents=\"3\" format=\"ascii\">\n";
    for (const model::Node &node : nodes.nodes())
    {
        output << "          "
               << analysis_result
                      .reactions[dof_map.global_index(node.id(), model::DisplacementComponent::x)]
               << ' '
               << analysis_result
                      .reactions[dof_map.global_index(node.id(), model::DisplacementComponent::y)]
               << ' '
               << analysis_result
                      .reactions[dof_map.global_index(node.id(), model::DisplacementComponent::z)]
               << '\n';
    }

    output << "        </DataArray>\n"
           << "      </PointData>\n"
           << "      <Points>\n"
           << "        <DataArray type=\"Float64\" NumberOfComponents=\"3\" "
              "format=\"ascii\">\n";
    for (const model::Node &node : nodes.nodes())
    {
        output << "          " << node.x() << ' ' << node.y() << ' ' << node.z() << '\n';
    }
    output << "        </DataArray>\n"
           << "      </Points>\n"
           << "      <Cells>\n"
           << "        <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n";
    for (const model::H8Element &element : elements.elements())
    {
        output << "          ";
        for (std::size_t node = 0; node < element.node_ids().size(); ++node)
        {
            if (node != 0)
            {
                output << ' ';
            }
            output << point_indices.at(element.node_ids()[node]);
        }
        output << '\n';
    }
    output << "        </DataArray>\n"
           << "        <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n"
           << "          ";
    for (std::size_t cell = 0; cell < elements.size(); ++cell)
    {
        if (cell != 0)
        {
            output << ' ';
        }
        output << 8 * (cell + 1);
    }
    output << "\n        </DataArray>\n"
           << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n"
           << "          ";
    for (std::size_t cell = 0; cell < elements.size(); ++cell)
    {
        if (cell != 0)
        {
            output << ' ';
        }
        output << 12;
    }
    output << "\n        </DataArray>\n"
           << "      </Cells>\n"
           << "    </Piece>\n"
           << "  </UnstructuredGrid>\n"
           << "</VTKFile>\n";
    return output.str();
}

void write_h8_analysis_vtu(const std::filesystem::path &path, const model::NodeCollection &nodes,
                           const model::H8ElementCollection &elements, const model::DofMap &dof_map,
                           const solver::H8AnalysisResult &analysis_result)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        throw std::runtime_error("Unable to open H8 VTU output file: " + path.string());
    }
    file << create_h8_analysis_vtu(nodes, elements, dof_map, analysis_result);
    if (!file)
    {
        throw std::runtime_error("Unable to write H8 VTU output file: " + path.string());
    }
}
} // namespace finelemethod::output
