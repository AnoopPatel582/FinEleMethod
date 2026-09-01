#include "finelemethod/input/abaqus_model_summary.hpp"

#include "finelemethod/input/abaqus_element_parser.hpp"
#include "finelemethod/input/abaqus_h8_model_parser.hpp"
#include "finelemethod/input/abaqus_q4_model_parser.hpp"

namespace finelemethod::input
{
AbaqusModelSummary summarize_abaqus_model(const std::string_view input_text)
{
    if (detect_abaqus_element_family(input_text) == AbaqusElementFamily::h8)
    {
        const AbaqusH8Model model = parse_abaqus_h8_model(input_text);
        return AbaqusModelSummary{
            .analysis_type = AbaqusAnalysisType::h8_three_dimensional,
            .node_count = model.nodes.size(),
            .element_count = model.elements.size(),
            .material_count = model.materials.size(),
            .prescribed_displacement_count = model.prescribed_displacements.size(),
            .point_load_count = model.point_loads.size(),
            .pressure_load_count = model.pressure_loads.size(),
        };
    }

    const AbaqusQ4Model model = parse_abaqus_q4_model(input_text);
    return AbaqusModelSummary{
        .analysis_type = model.analysis_type == Q4AnalysisType::plane_stress
                             ? AbaqusAnalysisType::q4_plane_stress
                             : AbaqusAnalysisType::q4_plane_strain,
        .node_count = model.nodes.size(),
        .element_count = model.elements.size(),
        .material_count = model.materials.size(),
        .prescribed_displacement_count = model.prescribed_displacements.size(),
        .point_load_count = model.point_loads.size(),
        .pressure_load_count = model.pressure_loads.size(),
    };
}
} // namespace finelemethod::input
