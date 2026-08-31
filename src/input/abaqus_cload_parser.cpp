#include "finelemethod/input/abaqus_cload_parser.hpp"

#include "abaqus_parser_utilities.hpp"

#include <cmath>
#include <cstddef>
#include <string>

namespace finelemethod::input
{
namespace
{
model::DisplacementComponent load_component(const std::size_t abaqus_dof,
                                            const model::SpatialDimension spatial_dimension,
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
    if (abaqus_dof == 3 && spatial_dimension == model::SpatialDimension::three_dimensional)
    {
        return model::DisplacementComponent::z;
    }
    throw AbaqusParseError("Unsupported concentrated-load degree of freedom for the model "
                           "dimension on line " +
                           std::to_string(line_number) + ".");
}
} // namespace

std::vector<AbaqusConcentratedLoad> parse_abaqus_concentrated_loads(
    const std::string_view input_text, const model::SpatialDimension spatial_dimension)
{
    std::vector<AbaqusConcentratedLoad> concentrated_loads;
    bool in_cload_section = false;
    bool found_cload_section = false;
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
            in_cload_section = detail::equals_case_insensitive(keyword, "CLOAD");
            found_cload_section = found_cload_section || in_cload_section;
        }
        else if (in_cload_section && !line.empty())
        {
            const auto fields = detail::split_fields(line);
            if (fields.size() != 3)
            {
                throw AbaqusParseError(
                    "Expected node, degree of freedom, and load magnitude on line " +
                    std::to_string(line_number) + ".");
            }

            const AbaqusNodeTarget target =
                detail::parse_node_target(fields[0], line_number, "Concentrated-load target");
            const std::size_t abaqus_dof =
                detail::parse_number<std::size_t>(fields[1], line_number, "concentrated-load DOF");
            const double magnitude =
                detail::parse_number<double>(fields[2], line_number, "concentrated-load magnitude");
            if (!std::isfinite(magnitude))
            {
                throw AbaqusParseError("Concentrated-load magnitude must be finite on line " +
                                       std::to_string(line_number) + ".");
            }

            concentrated_loads.push_back(AbaqusConcentratedLoad{
                target, load_component(abaqus_dof, spatial_dimension, line_number), magnitude});
        }

        if (line_end == std::string_view::npos)
        {
            break;
        }
        line_start = line_end + 1;
    }

    if (!found_cload_section)
    {
        throw AbaqusParseError("ABAQUS input does not contain a *CLOAD section.");
    }
    if (concentrated_loads.empty())
    {
        throw AbaqusParseError("ABAQUS *CLOAD section does not contain point-load data.");
    }
    return concentrated_loads;
}
} // namespace finelemethod::input
