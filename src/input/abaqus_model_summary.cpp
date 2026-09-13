#include "finelemethod/input/abaqus_model_summary.hpp"

#include "finelemethod/input/abaqus_cae_importer.hpp"

#include <stdexcept>

namespace finelemethod::input
{
AbaqusModelSummary summarize_abaqus_model(const std::string_view input_text)
{
    const AbaqusImportedModel model = import_abaqus_cae_model(input_text);
    AbaqusAnalysisType analysis_type = AbaqusAnalysisType::q4_plane_stress;
    switch (model.element_type)
    {
    case AbaqusElementType::cps3:
        analysis_type = AbaqusAnalysisType::t3_plane_stress;
        break;
    case AbaqusElementType::cps4:
        analysis_type = AbaqusAnalysisType::q4_plane_stress;
        break;
    case AbaqusElementType::cps4r:
        analysis_type = AbaqusAnalysisType::q4_reduced_plane_stress;
        break;
    case AbaqusElementType::cpe4:
        analysis_type = AbaqusAnalysisType::q4_plane_strain;
        break;
    case AbaqusElementType::c3d4:
        analysis_type = AbaqusAnalysisType::t4_three_dimensional;
        break;
    case AbaqusElementType::c3d8:
        analysis_type = AbaqusAnalysisType::h8_three_dimensional;
        break;
    case AbaqusElementType::c3d8r:
        analysis_type = AbaqusAnalysisType::h8_reduced_three_dimensional;
        break;
    default:
        throw std::logic_error("Unsupported imported ABAQUS element type.");
    }
    return AbaqusModelSummary{
        .analysis_type = analysis_type,
        .node_count = model.nodes.size(),
        .element_count = model.elements.size(),
        .material_count = model.material_count,
        .prescribed_displacement_count = model.prescribed_displacements.size(),
        .point_load_count = model.point_loads.size(),
        .pressure_load_count = model.pressure_loads.size(),
    };
}
} // namespace finelemethod::input
