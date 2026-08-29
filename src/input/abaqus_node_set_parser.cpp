#include "finelemethod/input/abaqus_node_set_parser.hpp"

#include "abaqus_parser_utilities.hpp"

#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace finelemethod::input
{
namespace
{
std::string uppercase_copy(const std::string_view text)
{
    std::string uppercase(text);
    for (char &character : uppercase)
    {
        character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    }
    return uppercase;
}

std::string require_node_set_name(const std::string_view line, const std::size_t line_number)
{
    std::optional<std::string> node_set_name;
    const auto fields = detail::split_fields(line);
    for (std::size_t field_index = 1; field_index < fields.size(); ++field_index)
    {
        if (detail::equals_case_insensitive(fields[field_index], "GENERATE"))
        {
            throw AbaqusParseError("ABAQUS *NSET, GENERATE is not supported on line " +
                                   std::to_string(line_number) + ".");
        }

        const std::size_t equals = fields[field_index].find('=');
        if (equals == std::string_view::npos)
        {
            continue;
        }
        const std::string_view name = detail::trim(fields[field_index].substr(0, equals));
        const std::string_view value = detail::trim(fields[field_index].substr(equals + 1));
        if (detail::equals_case_insensitive(name, "NSET"))
        {
            if (value.empty())
            {
                continue;
            }
            node_set_name = std::string(value);
        }
    }
    if (node_set_name.has_value())
    {
        return std::move(*node_set_name);
    }
    throw AbaqusParseError("ABAQUS *NSET keyword requires NSET on line " +
                           std::to_string(line_number) + ".");
}
} // namespace

std::vector<AbaqusNodeSet> parse_abaqus_node_sets(const std::string_view input_text)
{
    std::vector<AbaqusNodeSet> node_sets;
    std::vector<std::unordered_set<model::NodeId>> node_ids_by_set;
    std::unordered_map<std::string, std::size_t> set_indices_by_name;
    std::size_t current_set_index = 0;
    bool in_node_set_section = false;
    bool found_node_set_section = false;
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
            in_node_set_section = detail::equals_case_insensitive(keyword, "NSET");
            if (in_node_set_section)
            {
                found_node_set_section = true;
                std::string set_name = require_node_set_name(line, line_number);
                const std::string lookup_name = uppercase_copy(set_name);
                const auto existing_set = set_indices_by_name.find(lookup_name);
                if (existing_set == set_indices_by_name.end())
                {
                    current_set_index = node_sets.size();
                    set_indices_by_name.emplace(lookup_name, current_set_index);
                    node_sets.push_back(AbaqusNodeSet{std::move(set_name), {}});
                    node_ids_by_set.emplace_back();
                }
                else
                {
                    current_set_index = existing_set->second;
                }
            }
        }
        else if (in_node_set_section && !line.empty())
        {
            const auto fields = detail::split_fields(line);
            for (const std::string_view field : fields)
            {
                const model::NodeId node_id =
                    detail::parse_number<model::NodeId>(field, line_number, "node-set node ID");
                if (!node_ids_by_set[current_set_index].insert(node_id).second)
                {
                    throw AbaqusParseError("Duplicate node ID in ABAQUS node set on line " +
                                           std::to_string(line_number) + ".");
                }
                node_sets[current_set_index].node_ids.push_back(node_id);
            }
        }

        if (line_end == std::string_view::npos)
        {
            break;
        }
        line_start = line_end + 1;
    }

    if (!found_node_set_section)
    {
        throw AbaqusParseError("ABAQUS input does not contain an *NSET section.");
    }
    for (const AbaqusNodeSet &node_set : node_sets)
    {
        if (node_set.node_ids.empty())
        {
            throw AbaqusParseError("ABAQUS node set '" + node_set.name +
                                   "' does not contain nodes.");
        }
    }
    return node_sets;
}
} // namespace finelemethod::input
