#pragma once

#include "finelemethod/input/abaqus_node_target.hpp"
#include "finelemethod/input/abaqus_parse_error.hpp"

#include <cctype>
#include <charconv>
#include <cstddef>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace finelemethod::input::detail
{
inline std::string_view trim(const std::string_view text)
{
    std::size_t first = 0;
    while (first < text.size() && std::isspace(static_cast<unsigned char>(text[first])) != 0)
    {
        ++first;
    }
    std::size_t last = text.size();
    while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1])) != 0)
    {
        --last;
    }
    return text.substr(first, last - first);
}

inline bool equals_case_insensitive(const std::string_view left, const std::string_view right)
{
    if (left.size() != right.size())
    {
        return false;
    }
    for (std::size_t index = 0; index < left.size(); ++index)
    {
        if (std::toupper(static_cast<unsigned char>(left[index])) !=
            std::toupper(static_cast<unsigned char>(right[index])))
        {
            return false;
        }
    }
    return true;
}

inline std::vector<std::string_view> split_fields(const std::string_view line)
{
    std::vector<std::string_view> fields;
    std::size_t field_start = 0;
    while (field_start <= line.size())
    {
        const std::size_t comma = line.find(',', field_start);
        if (comma == std::string_view::npos)
        {
            fields.push_back(trim(line.substr(field_start)));
            break;
        }
        fields.push_back(trim(line.substr(field_start, comma - field_start)));
        field_start = comma + 1;
    }
    return fields;
}

inline AbaqusNodeTarget parse_node_target(const std::string_view field,
                                          const std::size_t line_number,
                                          const std::string_view description)
{
    model::NodeId node_id{};
    const char *const begin = field.data();
    const char *const end = begin + field.size();
    const auto [position, error] = std::from_chars(begin, end, node_id);
    if (!field.empty() && error == std::errc{} && position == end)
    {
        return node_id;
    }
    if (field.empty())
    {
        throw AbaqusParseError(std::string(description) + " is empty on line " +
                               std::to_string(line_number) + ".");
    }
    return std::string(field);
}

template <typename Number>
Number parse_number(const std::string_view field, const std::size_t line_number,
                    const std::string_view description)
{
    Number value{};
    const char *const begin = field.data();
    const char *const end = begin + field.size();
    const auto [position, error] = std::from_chars(begin, end, value);
    if (field.empty() || error != std::errc{} || position != end)
    {
        throw AbaqusParseError("Invalid " + std::string(description) + " on line " +
                               std::to_string(line_number) + ".");
    }
    return value;
}
} // namespace finelemethod::input::detail
