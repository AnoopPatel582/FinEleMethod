cmake_minimum_required(VERSION 3.25)

project(FinEleMethod LANGUAGES CXX)

option(
    FINELEMETHOD_ENABLE_MSVC_ANALYSIS
    "Run MSVC native code analysis and treat its findings as errors"
    OFF
)

include(CTest)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(wxWidgets CONFIG REQUIRED)

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    add_custom_target(
        api-docs
        COMMAND
            ${CMAKE_COMMAND} -E env
            "FINELEMETHOD_DOXYGEN_OUTPUT=${CMAKE_CURRENT_SOURCE_DIR}/out/docs/api"
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating FinEleMethod API documentation"
        VERBATIM
    )
endif()

add_library(FinEleMethodCore STATIC
    src/assembly/coo_assembly.cpp
    src/assembly/h8_dof_mapping.cpp
    src/assembly/h8_pressure_assembly.cpp
    src/assembly/h8_stiffness_assembly.cpp
    src/assembly/load_assembly.cpp
    src/assembly/q4_dof_mapping.cpp
    src/assembly/q4_pressure_assembly.cpp
    src/assembly/q4_stiffness_assembly.cpp
    src/core/application.cpp
    src/elements/h8_gauss_quadrature.cpp
    src/elements/h8_face_pressure.cpp
    src/elements/h8_jacobian.cpp
    src/elements/h8_physical_derivatives.cpp
    src/elements/h8_recovery.cpp
    src/elements/h8_shape_functions.cpp
    src/elements/h8_stiffness.cpp
    src/elements/h8_strain_displacement.cpp
    src/elements/q4_edge_pressure.cpp
    src/elements/q4_gauss_quadrature.cpp
    src/elements/q4_jacobian.cpp
    src/elements/q4_physical_derivatives.cpp
    src/elements/q4_plane_strain_recovery.cpp
    src/elements/q4_plane_strain_stiffness.cpp
    src/elements/q4_plane_stress_stiffness.cpp
    src/elements/q4_plane_stress_recovery.cpp
    src/elements/q4_shape_functions.cpp
    src/elements/q4_strain_displacement.cpp
    src/examples/h8_compression_example.cpp
    src/examples/q4_tension_example.cpp
    src/input/abaqus_boundary_parser.cpp
    src/input/abaqus_cload_parser.cpp
    src/input/abaqus_dload_parser.cpp
    src/input/abaqus_element_parser.cpp
    src/input/abaqus_h8_model_parser.cpp
    src/input/abaqus_input_file.cpp
    src/input/abaqus_material_parser.cpp
    src/input/abaqus_model_summary.cpp
    src/input/abaqus_node_parser.cpp
    src/input/abaqus_node_set_parser.cpp
    src/input/abaqus_q4_model_parser.cpp
    src/input/abaqus_solid_section_parser.cpp
    src/input/analysis_request.cpp
    src/math/coo_matrix.cpp
    src/math/csr_matrix.cpp
    src/math/dense_matrix.cpp
    src/math/dense_vector.cpp
    src/math/linear_solver.cpp
    src/mechanics/constitutive_matrix.cpp
    src/mechanics/plane_strain_measures.cpp
    src/mechanics/plane_stress_measures.cpp
    src/mechanics/solid_stress_measures.cpp
    src/model/dof_map.cpp
    src/model/h8_element.cpp
    src/model/h8_element_collection.cpp
    src/model/h8_face_pressure_load.cpp
    src/model/isotropic_elastic_material.cpp
    src/model/material_collection.cpp
    src/model/node.cpp
    src/model/node_collection.cpp
    src/model/point_load.cpp
    src/model/q4_edge_pressure_load.cpp
    src/model/q4_element.cpp
    src/model/q4_element_collection.cpp
    src/output/h8_analysis_vtu.cpp
    src/output/analysis_progress.cpp
    src/output/analysis_summary.cpp
    src/output/q4_analysis_vtu.cpp
    src/output/q4_vtu_writer.cpp
    src/postprocessing/h8_results.cpp
    src/postprocessing/q4_plane_strain_results.cpp
    src/postprocessing/q4_plane_stress_results.cpp
    src/project/analysis_run.cpp
    src/project/cancellation_flag.cpp
    src/project/project_file.cpp
    src/solver/abaqus_q4_analysis.cpp
    src/solver/abaqus_h8_analysis.cpp
    src/solver/abaqus_q4_plane_strain_analysis.cpp
    src/solver/abaqus_q4_plane_stress_analysis.cpp
    src/solver/boundary_conditions.cpp
    src/solver/conjugate_gradient.cpp
    src/solver/dense_static_solver.cpp
    src/solver/h8_analysis.cpp
    src/solver/q4_plane_strain_analysis.cpp
    src/solver/q4_plane_stress_analysis.cpp
    src/solver/reactions.cpp
    src/solver/sparse_static_solver.cpp
)

add_library(FinEleMethod::Core ALIAS FinEleMethodCore)

target_compile_features(FinEleMethodCore PUBLIC cxx_std_20)
target_include_directories(FinEleMethodCore
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(FinEleMethodCore PUBLIC nlohmann_json::nlohmann_json)

add_library(FinEleMethodCli STATIC
    src/cli/command_line.cpp
)

add_library(FinEleMethod::Cli ALIAS FinEleMethodCli)

target_compile_features(FinEleMethodCli PUBLIC cxx_std_20)
target_include_directories(FinEleMethodCli
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(FinEleMethodCli PUBLIC FinEleMethod::Core)

add_executable(FinEleMethod
    apps/cli/main.cpp
)

target_compile_features(FinEleMethod PRIVATE cxx_std_20)
target_link_libraries(FinEleMethod PRIVATE FinEleMethod::Cli)

add_executable(FinEleMethodGui WIN32
    apps/gui/main.cpp
    apps/gui/main_frame.cpp
    apps/gui/main_frame.hpp
)

target_compile_features(FinEleMethodGui PRIVATE cxx_std_20)
target_link_libraries(FinEleMethodGui PRIVATE FinEleMethod::Core wx::core wx::base)
add_dependencies(FinEleMethodGui FinEleMethod)

install(
    TARGETS FinEleMethod FinEleMethodGui
    RUNTIME DESTINATION .
)
install(
    DIRECTORY "$<TARGET_FILE_DIR:FinEleMethodGui>/"
    DESTINATION .
    FILES_MATCHING PATTERN "*.dll"
)
install(FILES README.md docs/BEGINNERS_GUIDE.md DESTINATION docs)
install(DIRECTORY examples/abaqus DESTINATION examples)

if(MSVC)
    target_compile_options(FinEleMethodCore PRIVATE /W4 /permissive-)
    target_compile_options(FinEleMethodCli PRIVATE /W4 /permissive-)
    target_compile_options(FinEleMethod PRIVATE /W4 /permissive-)
    target_compile_options(FinEleMethodGui PRIVATE /W4 /permissive-)

    if(FINELEMETHOD_ENABLE_MSVC_ANALYSIS)
        foreach(target FinEleMethodCore FinEleMethodCli FinEleMethod FinEleMethodGui)
            target_compile_options(${target} PRIVATE /analyze /WX)
        endforeach()
        # wxWidgets 3.3.3 triggers these analyzer warnings in wx/datetime.h.
        # Suppress only those third-party findings for the GUI target.
        target_compile_options(FinEleMethodGui PRIVATE /wd6235 /wd6240)
    endif()
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
