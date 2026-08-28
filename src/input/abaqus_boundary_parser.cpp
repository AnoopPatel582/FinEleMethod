#include "finelemethod/input/abaqus_boundary_parser.hpp"

#include "abaqus_parser_utilities.hpp"

#include <cmath>
#include <cstddef>
#include <string>
#include <unordered_set>

namespace finelemethod::input
{
namespace
{
model::DisplacementComponent displacement_component(const std::size_t abaqus_dof,
                                                    const std::size_t line_number)
{
    if (abaqus_dof == 1)
    {
        return model::DisplacementComponent::x;
    }
    if (abaqus_dof == 2)
    {
        return model::DisplacementComponent::y;
    }
    throw AbaqusParseError("Unsupported two-dimensional boundary degree of freedom on line " +
                           std::to_string(line_number) + ".");
}

void validate_boundary_keyword(const std::string_view line, const std::size_t line_number)
{
    const auto fields = detail::split_fields(line);
    for (std::size_t field_index = 1; field_index < fields.size(); ++field_index)
    {
        const std::size_t equals = fields[field_index].find('=');
        if (equals == std::string_view::npos)
        {
            continue;
        }
        const std::string_view name = detail::trim(fields[field_index].substr(0, equals));
        const std::string_view value = detail::trim(fields[field_index].substr(equals + 1));
        if (detail::equals_case_insensitive(name, "TYPE") &&
            !detail::equals_case_insensitive(value, "DISPLACEMENT"))
        {
            throw AbaqusParseError("Unsupported *BOUNDARY TYPE on line " +
                                   std::to_string(line_number) + ".");
        }
    }
}

} // namespace

std::vector<AbaqusNodalDisplacement> parse_abaqus_nodal_displacements(
    const std::string_view input_text)
{
    std::vector<AbaqusNodalDisplacement> displacements;
    std::unordered_set<model::NodeId> constrained_x_nodes;
    std::unordered_set<model::NodeId> constrained_y_nodes;
    bool in_boundary_section = false;
    bool found_boundary_section = false;
    std::size_t line_number = 0;
    std::size_t line_start = 0;

    while (line_start <= input_text.size())
    {
        ++line_number;
        const std::size_t line_end = input_text.find('\n', line_start);
        std::string_view line = line_end == std::string_view::npos
                                    ? input_text.substr(line_start)
                                    : input_text.substr(line_start, line_end - line_start);
        if (!line.empty() && line.back() == '\r')
        {
            line.remove_suffix(1);
        }
        line = detail::trim(line);

        if (line.starts_with("**"))
        {
            // ABAQUS comment line.
        }
        else if (line.starts_with('*'))
        {
            const std::size_t comma = line.find(',');
            const std::string_view keyword = detail::trim(
                line.substr(1, comma == std::string_view::npos ? line.size() - 1 : comma - 1));
            in_boundary_section = detail::equals_case_insensitive(keyword, "BOUNDARY");
            if (in_boundary_section)
            {
                found_boundary_section = true;
                validate_boundary_keyword(line, line_number);
            }
        }
        else if (in_boundary_section && !line.empty())
        {
            const auto fields = detail::split_fields(line);
            if (fields.size() < 2 || fields.size() > 4)
            {
                throw AbaqusParseError("Expected node, first DOF, optional last DOF, and optional "
                                       "value on line " +
                                       std::to_string(line_number) + ".");
            }

            const model::NodeId node_id =
                detail::parse_number<model::NodeId>(fields[0], line_number, "boundary node ID");
            const std::size_t first_dof =
                detail::parse_number<std::size_t>(fields[1], line_number, "first boundary DOF");
            const std::size_t last_dof =
                fields.size() >= 3
                    ? detail::parse_number<std::size_t>(fields[2], line_number, "last boundary DOF")
                    : first_dof;
            const double value = fields.size() == 4
                                     ? detail::parse_number<double>(fields[3], line_number,
                                                                    "prescribed displacement")
                                     : 0.0;
            if (first_dof > last_dof)
            {
                throw AbaqusParseError("First boundary DOF exceeds last boundary DOF on line " +
                                       std::to_string(line_number) + ".");
            }
            if (!std::isfinite(value))
            {
                throw AbaqusParseError("Prescribed displacement must be finite on line " +
                                       std::to_string(line_number) + ".");
            }

            for (std::size_t abaqus_dof = first_dof; abaqus_dof <= last_dof; ++abaqus_dof)
            {
                const model::DisplacementComponent component =
                    displacement_component(abaqus_dof, line_number);
                auto &constrained_nodes = component == model::DisplacementComponent::x
                                              ? constrained_x_nodes
                                              : constrained_y_nodes;
                if (!constrained_nodes.insert(node_id).second)
                {
                    throw AbaqusParseError("Duplicate prescribed displacement on line " +
                                           std::to_string(line_number) + ".");
                }
                displacements.push_back(AbaqusNodalDisplacement{node_id, component, value});
            }
        }

        if (line_end == std::string_view::npos)
        {
            break;
        }
        line_start = line_end + 1;
    }

    if (!found_boundary_section)
    {
        throw AbaqusParseError("ABAQUS input does not contain a *BOUNDARY section.");
    }
    if (displacements.empty())
    {
        throw AbaqusParseError("ABAQUS *BOUNDARY section does not contain displacement data.");
    }
    return displacements;
}
} // namespace finelemethod::input
