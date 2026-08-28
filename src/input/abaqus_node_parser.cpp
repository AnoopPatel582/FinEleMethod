#include "finelemethod/input/abaqus_node_parser.hpp"

#include "finelemethod/model/node.hpp"

#include <cctype>
#include <charconv>
#include <cstddef>
#include <string>
#include <system_error>
#include <vector>

namespace finelemethod::input
{
namespace
{
std::string_view trim(const std::string_view text)
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

bool equals_case_insensitive(const std::string_view left, const std::string_view right)
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

std::vector<std::string_view> split_fields(const std::string_view line)
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
} // namespace

model::NodeCollection parse_abaqus_nodes(const std::string_view input_text)
{
    model::NodeCollection nodes;
    bool in_node_section = false;
    bool found_node_section = false;
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
        line = trim(line);

        if (line.starts_with("**"))
        {
            // ABAQUS comment line.
        }
        else if (line.starts_with('*'))
        {
            const std::size_t comma = line.find(',');
            const std::string_view keyword =
                trim(line.substr(1, comma == std::string_view::npos ? line.size() - 1 : comma - 1));
            in_node_section = equals_case_insensitive(keyword, "NODE");
            found_node_section = found_node_section || in_node_section;
        }
        else if (in_node_section && !line.empty())
        {
            const auto fields = split_fields(line);
            if (fields.size() != 3 && fields.size() != 4)
            {
                throw AbaqusParseError("Expected node ID, x, y, and optional z on line " +
                                       std::to_string(line_number) + ".");
            }

            const model::NodeId id = parse_number<model::NodeId>(fields[0], line_number, "node ID");
            const double x = parse_number<double>(fields[1], line_number, "x coordinate");
            const double y = parse_number<double>(fields[2], line_number, "y coordinate");
            const double z = fields.size() == 4
                                 ? parse_number<double>(fields[3], line_number, "z coordinate")
                                 : 0.0;
            try
            {
                nodes.add(model::Node(id, x, y, z));
            }
            catch (const std::exception &exception)
            {
                throw AbaqusParseError("Invalid node on line " + std::to_string(line_number) +
                                       ": " + exception.what());
            }
        }

        if (line_end == std::string_view::npos)
        {
            break;
        }
        line_start = line_end + 1;
    }

    if (!found_node_section)
    {
        throw AbaqusParseError("ABAQUS input does not contain a *NODE section.");
    }
    if (nodes.empty())
    {
        throw AbaqusParseError("ABAQUS *NODE section does not contain any nodes.");
    }
    return nodes;
}
} // namespace finelemethod::input
