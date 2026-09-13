#include "finelemethod/output/abaqus_imported_vtu.hpp"

#include "finelemethod/input/abaqus_cae_importer.hpp"

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace finelemethod::output
{
namespace
{
std::size_t vtk_cell_type(const input::AbaqusElementType type)
{
    switch (type)
    {
    case input::AbaqusElementType::cps3:
        return 5;
    case input::AbaqusElementType::cps4:
    case input::AbaqusElementType::cps4r:
    case input::AbaqusElementType::cpe4:
        return 9;
    case input::AbaqusElementType::c3d4:
        return 10;
    case input::AbaqusElementType::c3d8:
    case input::AbaqusElementType::c3d8r:
        return 12;
    }
    throw std::invalid_argument("Unsupported ABAQUS element type for VTU output.");
}

void validate_solution(const solver::AbaqusImportedAnalysisSolution &solution)
{
    const std::size_t dimension = input::abaqus_element_dimension(solution.model.element_type);
    const std::size_t dof_count = solution.model.nodes.size() * dimension;
    if (solution.model.nodes.empty() || solution.model.elements.empty())
    {
        throw std::invalid_argument("Imported VTU output requires nodes and elements.");
    }
    if (solution.displacements.size() != dof_count || solution.reactions.size() != dof_count)
    {
        throw std::invalid_argument("Imported VTU vectors do not match the model dimension.");
    }
    if (solution.element_results.size() != solution.model.elements.size())
    {
        throw std::invalid_argument("Imported VTU element results do not match the model.");
    }
    for (std::size_t index = 0; index < solution.model.elements.size(); ++index)
    {
        if (solution.element_results[index].element_id != solution.model.elements[index].id)
        {
            throw std::invalid_argument("Imported VTU results must follow element order.");
        }
    }
}

template <std::size_t Size>
void write_array(std::ostringstream &output, const std::array<double, Size> &values,
                 const std::size_t count)
{
    for (std::size_t index = 0; index < count; ++index)
    {
        if (index != 0)
        {
            output << ' ';
        }
        output << values[index];
    }
    output << '\n';
}
} // namespace

std::string create_abaqus_imported_vtu(const solver::AbaqusImportedAnalysisSolution &solution)
{
    validate_solution(solution);
    const std::size_t dimension = input::abaqus_element_dimension(solution.model.element_type);
    const std::size_t tensor_components = dimension == 2 ? 3 : 6;
    std::unordered_map<model::NodeId, std::size_t> point_indices;
    point_indices.reserve(solution.model.nodes.size());
    for (std::size_t index = 0; index < solution.model.nodes.size(); ++index)
    {
        point_indices.emplace(solution.model.nodes[index].id, index);
    }

    std::ostringstream output;
    output << std::setprecision(17) << std::scientific;
    output << "<?xml version=\"1.0\"?>\n"
           << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" "
              "byte_order=\"LittleEndian\">\n"
           << "  <UnstructuredGrid>\n"
           << "    <Piece NumberOfPoints=\"" << solution.model.nodes.size() << "\" NumberOfCells=\""
           << solution.model.elements.size() << "\">\n"
           << "      <CellData Scalars=\"VonMises\">\n"
           << "        <DataArray type=\"Float64\" Name=\"Strain\" NumberOfComponents=\""
           << tensor_components << "\" format=\"ascii\">\n";
    for (const auto &result : solution.element_results)
    {
        output << "          ";
        if (dimension == 2)
        {
            const std::array<double, 3> values{
                {result.strain[0], result.strain[1], result.strain[3]}};
            write_array(output, values, values.size());
        }
        else
        {
            write_array(output, result.strain, result.strain.size());
        }
    }
    output << "        </DataArray>\n"
           << "        <DataArray type=\"Float64\" Name=\"Stress\" NumberOfComponents=\""
           << tensor_components << "\" format=\"ascii\">\n";
    for (const auto &result : solution.element_results)
    {
        output << "          ";
        if (dimension == 2)
        {
            const std::array<double, 3> values{
                {result.stress[0], result.stress[1], result.stress[3]}};
            write_array(output, values, values.size());
        }
        else
        {
            write_array(output, result.stress, result.stress.size());
        }
    }
    output << "        </DataArray>\n"
           << "        <DataArray type=\"Float64\" Name=\"VonMises\" format=\"ascii\">\n";
    for (const auto &result : solution.element_results)
    {
        output << "          " << result.von_mises << '\n';
    }
    output << "        </DataArray>\n"
           << "        <DataArray type=\"Float64\" Name=\"PrincipalStress\" "
              "NumberOfComponents=\"3\" format=\"ascii\">\n";
    for (const auto &result : solution.element_results)
    {
        output << "          ";
        write_array(output, result.principal_stresses, result.principal_stresses.size());
    }
    output << "        </DataArray>\n"
           << "      </CellData>\n"
           << "      <PointData Vectors=\"Displacement\">\n"
           << "        <DataArray type=\"Float64\" Name=\"Displacement\" "
              "NumberOfComponents=\"3\" format=\"ascii\">\n";
    for (std::size_t node = 0; node < solution.model.nodes.size(); ++node)
    {
        output << "          ";
        for (std::size_t component = 0; component < 3; ++component)
        {
            if (component != 0)
            {
                output << ' ';
            }
            output << (component < dimension ? solution.displacements[node * dimension + component]
                                             : 0.0);
        }
        output << '\n';
    }
    output << "        </DataArray>\n"
           << "        <DataArray type=\"Float64\" Name=\"ReactionForce\" "
              "NumberOfComponents=\"3\" format=\"ascii\">\n";
    for (std::size_t node = 0; node < solution.model.nodes.size(); ++node)
    {
        output << "          ";
        for (std::size_t component = 0; component < 3; ++component)
        {
            if (component != 0)
            {
                output << ' ';
            }
            output << (component < dimension ? solution.reactions[node * dimension + component]
                                             : 0.0);
        }
        output << '\n';
    }
    output << "        </DataArray>\n"
           << "      </PointData>\n"
           << "      <Points>\n"
           << "        <DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">\n";
    for (const auto &node : solution.model.nodes)
    {
        output << "          " << node.coordinates[0] << ' ' << node.coordinates[1] << ' '
               << node.coordinates[2] << '\n';
    }
    output << "        </DataArray>\n"
           << "      </Points>\n"
           << "      <Cells>\n"
           << "        <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n";
    for (const auto &element : solution.model.elements)
    {
        output << "          ";
        for (std::size_t node = 0; node < element.node_ids.size(); ++node)
        {
            if (node != 0)
            {
                output << ' ';
            }
            output << point_indices.at(element.node_ids[node]);
        }
        output << '\n';
    }
    output << "        </DataArray>\n"
           << "        <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n"
           << "          ";
    std::size_t offset = 0;
    for (std::size_t index = 0; index < solution.model.elements.size(); ++index)
    {
        if (index != 0)
        {
            output << ' ';
        }
        offset += solution.model.elements[index].node_ids.size();
        output << offset;
    }
    output << "\n        </DataArray>\n"
           << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n"
           << "          ";
    for (std::size_t index = 0; index < solution.model.elements.size(); ++index)
    {
        if (index != 0)
        {
            output << ' ';
        }
        output << vtk_cell_type(solution.model.elements[index].type);
    }
    output << "\n        </DataArray>\n"
           << "      </Cells>\n"
           << "    </Piece>\n"
           << "  </UnstructuredGrid>\n"
           << "</VTKFile>\n";
    return output.str();
}

void write_abaqus_imported_vtu(const std::filesystem::path &path,
                               const solver::AbaqusImportedAnalysisSolution &solution)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        throw std::runtime_error("Unable to open VTU output file: " + path.string());
    }
    file << create_abaqus_imported_vtu(solution);
    if (!file)
    {
        throw std::runtime_error("Unable to write VTU output file: " + path.string());
    }
}
} // namespace finelemethod::output
