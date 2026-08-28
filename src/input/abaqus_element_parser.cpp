#include "finelemethod/input/abaqus_element_parser.hpp"

#include "abaqus_parser_utilities.hpp"

#include <cstddef>
#include <string>
#include <unordered_set>

namespace finelemethod::input
{
namespace
{
struct ElementSection
{
    bool is_q4 = false;
    std::string element_set;
};

ElementSection parse_element_keyword(const std::string_view line, const std::size_t line_number)
{
    ElementSection section;
    bool found_type = false;
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
        if (detail::equals_case_insensitive(name, "TYPE"))
        {
            found_type = true;
            section.is_q4 = detail::equals_case_insensitive(value, "CPS4");
        }
        else if (detail::equals_case_insensitive(name, "ELSET"))
        {
            section.element_set = std::string(value);
        }
    }
    if (!found_type)
    {
        throw AbaqusParseError("ABAQUS *ELEMENT keyword requires TYPE on line " +
                               std::to_string(line_number) + ".");
    }
    return section;
}
} // namespace

std::vector<AbaqusQ4Element> parse_abaqus_q4_elements(const std::string_view input_text)
{
    std::vector<AbaqusQ4Element> elements;
    std::unordered_set<model::ElementId> element_ids;
    ElementSection section;
    bool in_element_section = false;
    bool found_element_section = false;
    bool found_q4_section = false;
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
            in_element_section = detail::equals_case_insensitive(keyword, "ELEMENT");
            if (in_element_section)
            {
                found_element_section = true;
                section = parse_element_keyword(line, line_number);
                found_q4_section = found_q4_section || section.is_q4;
            }
        }
        else if (in_element_section && section.is_q4 && !line.empty())
        {
            const auto fields = detail::split_fields(line);
            if (fields.size() != 5)
            {
                throw AbaqusParseError("Expected element ID and four node IDs on line " +
                                       std::to_string(line_number) + ".");
            }

            const model::ElementId id =
                detail::parse_number<model::ElementId>(fields[0], line_number, "element ID");
            if (!element_ids.insert(id).second)
            {
                throw AbaqusParseError("Duplicate Q4 element ID on line " +
                                       std::to_string(line_number) + ".");
            }
            model::Q4NodeIds node_ids{};
            for (std::size_t node_index = 0; node_index < node_ids.size(); ++node_index)
            {
                node_ids[node_index] = detail::parse_number<model::NodeId>(
                    fields[node_index + 1], line_number, "element node ID");
            }
            elements.push_back(AbaqusQ4Element{id, node_ids, section.element_set});
        }

        if (line_end == std::string_view::npos)
        {
            break;
        }
        line_start = line_end + 1;
    }

    if (!found_element_section)
    {
        throw AbaqusParseError("ABAQUS input does not contain an *ELEMENT section.");
    }
    if (!found_q4_section)
    {
        throw AbaqusParseError("ABAQUS input does not contain supported *ELEMENT, TYPE=CPS4 data.");
    }
    if (elements.empty())
    {
        throw AbaqusParseError("ABAQUS CPS4 element section does not contain any elements.");
    }
    return elements;
}
} // namespace finelemethod::input
