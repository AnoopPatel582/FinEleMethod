#include "finelemethod/input/abaqus_solid_section_parser.hpp"

#include "abaqus_parser_utilities.hpp"

#include <cctype>
#include <cmath>
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>

namespace finelemethod::input
{
namespace
{
struct PendingSection
{
    std::string element_set;
    std::string material_name;
    std::size_t keyword_line;
};

std::string uppercase_copy(const std::string_view text)
{
    std::string uppercase(text);
    for (char &character : uppercase)
    {
        character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    }
    return uppercase;
}

PendingSection parse_solid_section_keyword(const std::string_view line,
                                           const std::size_t line_number)
{
    PendingSection section{{}, {}, line_number};
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
        if (detail::equals_case_insensitive(name, "ELSET"))
        {
            section.element_set = std::string(value);
        }
        else if (detail::equals_case_insensitive(name, "MATERIAL"))
        {
            section.material_name = std::string(value);
        }
    }
    if (section.element_set.empty())
    {
        throw AbaqusParseError("ABAQUS *SOLID SECTION requires ELSET on line " +
                               std::to_string(line_number) + ".");
    }
    if (section.material_name.empty())
    {
        throw AbaqusParseError("ABAQUS *SOLID SECTION requires MATERIAL on line " +
                               std::to_string(line_number) + ".");
    }
    return section;
}
} // namespace

std::vector<AbaqusSolidSection> parse_abaqus_solid_sections(const std::string_view input_text)
{
    std::vector<AbaqusSolidSection> sections;
    std::unordered_set<std::string> assigned_element_sets;
    std::optional<PendingSection> pending_section;
    bool found_solid_section = false;
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
            if (pending_section.has_value())
            {
                throw AbaqusParseError("Missing solid-section thickness after line " +
                                       std::to_string(pending_section->keyword_line) + ".");
            }

            const std::size_t comma = line.find(',');
            const std::string_view keyword = detail::trim(
                line.substr(1, comma == std::string_view::npos ? line.size() - 1 : comma - 1));
            if (detail::equals_case_insensitive(keyword, "SOLID SECTION"))
            {
                found_solid_section = true;
                pending_section = parse_solid_section_keyword(line, line_number);
                if (!assigned_element_sets.insert(uppercase_copy(pending_section->element_set))
                         .second)
                {
                    throw AbaqusParseError("Duplicate solid section for element set on line " +
                                           std::to_string(line_number) + ".");
                }
            }
        }
        else if (pending_section.has_value() && !line.empty())
        {
            const auto fields = detail::split_fields(line);
            if (fields.empty() || fields[0].empty())
            {
                throw AbaqusParseError("Missing solid-section thickness on line " +
                                       std::to_string(line_number) + ".");
            }
            for (std::size_t field_index = 1; field_index < fields.size(); ++field_index)
            {
                if (!fields[field_index].empty())
                {
                    throw AbaqusParseError("Unsupported solid-section data on line " +
                                           std::to_string(line_number) + ".");
                }
            }
            const double thickness =
                detail::parse_number<double>(fields[0], line_number, "solid-section thickness");
            if (!std::isfinite(thickness) || thickness <= 0.0)
            {
                throw AbaqusParseError(
                    "Solid-section thickness must be finite and positive on line " +
                    std::to_string(line_number) + ".");
            }
            sections.push_back(AbaqusSolidSection{std::move(pending_section->element_set),
                                                  std::move(pending_section->material_name),
                                                  thickness});
            pending_section.reset();
        }

        if (line_end == std::string_view::npos)
        {
            break;
        }
        line_start = line_end + 1;
    }

    if (pending_section.has_value())
    {
        throw AbaqusParseError("Missing solid-section thickness after line " +
                               std::to_string(pending_section->keyword_line) + ".");
    }
    if (!found_solid_section)
    {
        throw AbaqusParseError("ABAQUS input does not contain a *SOLID SECTION definition.");
    }
    return sections;
}
} // namespace finelemethod::input
