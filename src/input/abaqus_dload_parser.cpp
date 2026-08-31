#include "finelemethod/input/abaqus_dload_parser.hpp"

#include "abaqus_parser_utilities.hpp"

#include <charconv>
#include <cmath>
#include <cstddef>
#include <string>
#include <system_error>

namespace finelemethod::input
{
namespace
{
AbaqusElementTarget parse_element_target(const std::string_view field,
                                         const std::size_t line_number)
{
    model::ElementId element_id{};
    const char *const begin = field.data();
    const char *const end = begin + field.size();
    const auto [position, error] = std::from_chars(begin, end, element_id);
    if (!field.empty() && error == std::errc{} && position == end)
    {
        return element_id;
    }
    if (field.empty())
    {
        throw AbaqusParseError("Distributed-load target is empty on line " +
                               std::to_string(line_number) + ".");
    }
    return std::string(field);
}

model::Q4Edge parse_edge_label(const std::string_view label, const std::size_t line_number)
{
    if (detail::equals_case_insensitive(label, "P1"))
    {
        return model::Q4Edge::one;
    }
    if (detail::equals_case_insensitive(label, "P2"))
    {
        return model::Q4Edge::two;
    }
    if (detail::equals_case_insensitive(label, "P3"))
    {
        return model::Q4Edge::three;
    }
    if (detail::equals_case_insensitive(label, "P4"))
    {
        return model::Q4Edge::four;
    }
    throw AbaqusParseError("Unsupported Q4 distributed-load type on line " +
                           std::to_string(line_number) + ".");
}

model::H8Face parse_face_label(const std::string_view label, const std::size_t line_number)
{
    if (detail::equals_case_insensitive(label, "P1"))
    {
        return model::H8Face::one;
    }
    if (detail::equals_case_insensitive(label, "P2"))
    {
        return model::H8Face::two;
    }
    if (detail::equals_case_insensitive(label, "P3"))
    {
        return model::H8Face::three;
    }
    if (detail::equals_case_insensitive(label, "P4"))
    {
        return model::H8Face::four;
    }
    if (detail::equals_case_insensitive(label, "P5"))
    {
        return model::H8Face::five;
    }
    if (detail::equals_case_insensitive(label, "P6"))
    {
        return model::H8Face::six;
    }
    throw AbaqusParseError("Unsupported H8 distributed-load type on line " +
                           std::to_string(line_number) + ".");
}

template <typename PressureLoad, typename LabelParser>
std::vector<PressureLoad> parse_pressure_loads(const std::string_view input_text,
                                               LabelParser parse_label,
                                               const std::string_view result_description)
{
    std::vector<PressureLoad> pressure_loads;
    bool in_dload_section = false;
    bool found_dload_section = false;
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
            in_dload_section = detail::equals_case_insensitive(keyword, "DLOAD");
            if (in_dload_section && detail::split_fields(line).size() != 1)
            {
                throw AbaqusParseError("ABAQUS *DLOAD parameters are not supported on line " +
                                       std::to_string(line_number) + ".");
            }
            found_dload_section = found_dload_section || in_dload_section;
        }
        else if (in_dload_section && !line.empty())
        {
            const auto fields = detail::split_fields(line);
            if (fields.size() != 3)
            {
                throw AbaqusParseError("Expected element, pressure face, and magnitude on line " +
                                       std::to_string(line_number) + ".");
            }

            const AbaqusElementTarget target = parse_element_target(fields[0], line_number);
            const auto label = parse_label(fields[1], line_number);
            const double pressure =
                detail::parse_number<double>(fields[2], line_number, "pressure magnitude");
            if (!std::isfinite(pressure))
            {
                throw AbaqusParseError("Pressure magnitude must be finite on line " +
                                       std::to_string(line_number) + ".");
            }
            pressure_loads.push_back(PressureLoad{target, label, pressure});
        }

        if (line_end == std::string_view::npos)
        {
            break;
        }
        line_start = line_end + 1;
    }

    if (!found_dload_section)
    {
        throw AbaqusParseError("ABAQUS input does not contain a *DLOAD section.");
    }
    if (pressure_loads.empty())
    {
        throw AbaqusParseError("ABAQUS *DLOAD section does not contain " +
                               std::string(result_description) + " data.");
    }
    return pressure_loads;
}
} // namespace

std::vector<AbaqusQ4EdgePressure> parse_abaqus_q4_edge_pressures(const std::string_view input_text)
{
    return parse_pressure_loads<AbaqusQ4EdgePressure>(input_text, parse_edge_label,
                                                      "edge-pressure");
}

std::vector<AbaqusH8FacePressure> parse_abaqus_h8_face_pressures(const std::string_view input_text)
{
    return parse_pressure_loads<AbaqusH8FacePressure>(input_text, parse_face_label,
                                                      "face-pressure");
}
} // namespace finelemethod::input
