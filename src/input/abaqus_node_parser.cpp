#include "finelemethod/input/abaqus_node_parser.hpp"

#include "abaqus_parser_utilities.hpp"
#include "finelemethod/model/node.hpp"

#include <cstddef>
#include <string>

namespace finelemethod::input
{
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
            in_node_section = detail::equals_case_insensitive(keyword, "NODE");
            found_node_section = found_node_section || in_node_section;
        }
        else if (in_node_section && !line.empty())
        {
            const auto fields = detail::split_fields(line);
            if (fields.size() != 3 && fields.size() != 4)
            {
                throw AbaqusParseError("Expected node ID, x, y, and optional z on line " +
                                       std::to_string(line_number) + ".");
            }

            const model::NodeId id =
                detail::parse_number<model::NodeId>(fields[0], line_number, "node ID");
            const double x = detail::parse_number<double>(fields[1], line_number, "x coordinate");
            const double y = detail::parse_number<double>(fields[2], line_number, "y coordinate");
            const double z =
                fields.size() == 4
                    ? detail::parse_number<double>(fields[3], line_number, "z coordinate")
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
