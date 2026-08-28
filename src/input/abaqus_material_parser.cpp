#include "finelemethod/input/abaqus_material_parser.hpp"

#include "abaqus_parser_utilities.hpp"
#include "finelemethod/model/isotropic_elastic_material.hpp"

#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>

namespace finelemethod::input
{
namespace
{
struct PendingMaterial
{
    std::string name;
    std::optional<double> youngs_modulus;
    std::optional<double> poisson_ratio;
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

std::string require_material_name(const std::string_view line, const std::size_t line_number)
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
        if (detail::equals_case_insensitive(name, "NAME"))
        {
            if (value.empty())
            {
                break;
            }
            return std::string(value);
        }
    }
    throw AbaqusParseError("ABAQUS *MATERIAL keyword requires NAME on line " +
                           std::to_string(line_number) + ".");
}

void validate_elastic_keyword(const std::string_view line, const std::size_t line_number)
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
            !detail::equals_case_insensitive(value, "ISOTROPIC"))
        {
            throw AbaqusParseError("Unsupported *ELASTIC TYPE on line " +
                                   std::to_string(line_number) + ".");
        }
    }
}
} // namespace

std::vector<AbaqusIsotropicMaterial> parse_abaqus_isotropic_materials(
    const std::string_view input_text)
{
    std::vector<PendingMaterial> pending_materials;
    std::unordered_set<std::string> material_names;
    std::optional<std::size_t> current_material;
    bool awaiting_elastic_data = false;
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
            awaiting_elastic_data = false;

            if (detail::equals_case_insensitive(keyword, "MATERIAL"))
            {
                std::string name = require_material_name(line, line_number);
                if (!material_names.insert(uppercase_copy(name)).second)
                {
                    throw AbaqusParseError("Duplicate ABAQUS material name on line " +
                                           std::to_string(line_number) + ".");
                }
                pending_materials.push_back(PendingMaterial{std::move(name), {}, {}});
                current_material = pending_materials.size() - 1;
            }
            else if (detail::equals_case_insensitive(keyword, "ELASTIC"))
            {
                if (!current_material.has_value())
                {
                    throw AbaqusParseError("ABAQUS *ELASTIC appears before *MATERIAL on line " +
                                           std::to_string(line_number) + ".");
                }
                if (pending_materials[*current_material].youngs_modulus.has_value())
                {
                    throw AbaqusParseError("Duplicate *ELASTIC data for material on line " +
                                           std::to_string(line_number) + ".");
                }
                validate_elastic_keyword(line, line_number);
                awaiting_elastic_data = true;
            }
        }
        else if (awaiting_elastic_data && !line.empty())
        {
            const auto fields = detail::split_fields(line);
            if (fields.size() != 2)
            {
                throw AbaqusParseError("Expected Young's modulus and Poisson ratio on line " +
                                       std::to_string(line_number) + ".");
            }
            const double youngs_modulus =
                detail::parse_number<double>(fields[0], line_number, "Young's modulus");
            const double poisson_ratio =
                detail::parse_number<double>(fields[1], line_number, "Poisson ratio");
            try
            {
                static_cast<void>(
                    model::IsotropicElasticMaterial(1, youngs_modulus, poisson_ratio));
            }
            catch (const std::exception &exception)
            {
                throw AbaqusParseError("Invalid elastic data on line " +
                                       std::to_string(line_number) + ": " + exception.what());
            }
            pending_materials[*current_material].youngs_modulus = youngs_modulus;
            pending_materials[*current_material].poisson_ratio = poisson_ratio;
            awaiting_elastic_data = false;
        }

        if (line_end == std::string_view::npos)
        {
            break;
        }
        line_start = line_end + 1;
    }

    if (pending_materials.empty())
    {
        throw AbaqusParseError("ABAQUS input does not contain a *MATERIAL definition.");
    }

    std::vector<AbaqusIsotropicMaterial> materials;
    materials.reserve(pending_materials.size());
    for (const PendingMaterial &material : pending_materials)
    {
        if (!material.youngs_modulus.has_value())
        {
            throw AbaqusParseError("ABAQUS material '" + material.name +
                                   "' does not contain isotropic *ELASTIC data.");
        }
        materials.push_back(AbaqusIsotropicMaterial{material.name, *material.youngs_modulus,
                                                    *material.poisson_ratio});
    }
    return materials;
}
} // namespace finelemethod::input
