find_package(GTest CONFIG REQUIRED)

add_executable(FinEleMethodTests
    assembly/h8_dof_mapping_test.cpp
    assembly/h8_stiffness_assembly_test.cpp
    assembly/load_assembly_test.cpp
    assembly/q4_dof_mapping_test.cpp
    assembly/q4_pressure_assembly_test.cpp
    assembly/q4_plane_strain_stiffness_assembly_test.cpp
    assembly/q4_stiffness_assembly_test.cpp
    cli/command_line_test.cpp
    core/application_test.cpp
    core/exit_code_test.cpp
    elements/h8_gauss_quadrature_test.cpp
    elements/h8_jacobian_test.cpp
    elements/h8_model_stiffness_test.cpp
    elements/h8_physical_derivatives_test.cpp
    elements/h8_recovery_test.cpp
    elements/h8_shape_functions_test.cpp
    elements/h8_stiffness_test.cpp
    elements/h8_strain_displacement_test.cpp
    elements/q4_edge_pressure_test.cpp
    elements/q4_gauss_quadrature_test.cpp
    elements/q4_jacobian_test.cpp
    elements/q4_physical_derivatives_test.cpp
    elements/q4_plane_strain_model_stiffness_test.cpp
    elements/q4_plane_strain_recovery_test.cpp
    elements/q4_plane_strain_stiffness_test.cpp
    elements/q4_plane_stress_model_stiffness_test.cpp
    elements/q4_plane_stress_recovery_test.cpp
    elements/q4_plane_stress_stiffness_test.cpp
    elements/q4_shape_functions_test.cpp
    elements/q4_strain_displacement_test.cpp
    input/abaqus_boundary_parser_test.cpp
    input/abaqus_cload_parser_test.cpp
    input/abaqus_dload_parser_test.cpp
    input/abaqus_element_parser_test.cpp
    input/abaqus_h8_model_parser_test.cpp
    input/abaqus_input_file_test.cpp
    input/abaqus_material_parser_test.cpp
    input/abaqus_node_parser_test.cpp
    input/abaqus_node_set_parser_test.cpp
    input/abaqus_q4_model_parser_test.cpp
    input/abaqus_solid_section_parser_test.cpp
    math/dense_matrix_test.cpp
    math/dense_vector_test.cpp
    math/linear_solver_test.cpp
    mechanics/constitutive_matrix_test.cpp
    mechanics/plane_strain_measures_test.cpp
    mechanics/plane_stress_measures_test.cpp
    mechanics/solid_stress_measures_test.cpp
    model/dof_map_test.cpp
    model/h8_element_test.cpp
    model/h8_element_collection_test.cpp
    model/isotropic_elastic_material_test.cpp
    model/material_collection_test.cpp
    model/node_test.cpp
    model/node_collection_test.cpp
    model/point_load_test.cpp
    model/q4_edge_pressure_load_test.cpp
    model/q4_element_test.cpp
    model/q4_element_collection_test.cpp
    output/h8_analysis_vtu_test.cpp
    output/q4_analysis_vtu_test.cpp
    output/q4_vtu_writer_test.cpp
    postprocessing/h8_results_test.cpp
    postprocessing/q4_plane_strain_results_test.cpp
    postprocessing/q4_plane_stress_results_test.cpp
    solver/abaqus_q4_analysis_test.cpp
    solver/abaqus_q4_plane_strain_analysis_test.cpp
    solver/abaqus_q4_plane_stress_analysis_test.cpp
    solver/boundary_conditions_test.cpp
    solver/dense_static_solver_test.cpp
    solver/h8_analysis_test.cpp
    solver/q4_plane_strain_analysis_test.cpp
    solver/q4_plane_stress_analysis_test.cpp
    solver/reactions_test.cpp
)

target_compile_features(FinEleMethodTests PRIVATE cxx_std_20)
target_link_libraries(FinEleMethodTests
    PRIVATE
        FinEleMethod::Core
        FinEleMethod::Cli
        GTest::gtest_main
)

if(MSVC)
    target_compile_options(FinEleMethodTests PRIVATE /W4 /permissive-)
endif()

include(GoogleTest)
gtest_discover_tests(FinEleMethodTests)

add_test(
    NAME CommandLineExample.Q4TensionInputRuns
    COMMAND FinEleMethod
        --input ${PROJECT_SOURCE_DIR}/examples/abaqus/q4_tension.inp
        --output ${PROJECT_BINARY_DIR}/q4_tension_example.vtu
)

add_test(
    NAME CommandLineExample.H8CompressionRuns
    COMMAND FinEleMethod --example h8-compression
        --output ${PROJECT_BINARY_DIR}/h8_compression_example.vtu
)

add_test(
    NAME CommandLineExample.Q4PlaneStrainTensionInputRuns
    COMMAND FinEleMethod
        --input ${PROJECT_SOURCE_DIR}/examples/abaqus/q4_plane_strain_tension.inp
        --output ${PROJECT_BINARY_DIR}/q4_plane_strain_tension_example.vtu
)
