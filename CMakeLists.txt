cmake_minimum_required(VERSION 3.25)

project(FinEleMethod LANGUAGES CXX)

include(CTest)

add_library(FinEleMethodCore STATIC
    src/assembly/load_assembly.cpp
    src/assembly/q4_dof_mapping.cpp
    src/assembly/q4_pressure_assembly.cpp
    src/assembly/q4_stiffness_assembly.cpp
    src/core/application.cpp
    src/elements/h8_gauss_quadrature.cpp
    src/elements/h8_jacobian.cpp
    src/elements/h8_physical_derivatives.cpp
    src/elements/h8_shape_functions.cpp
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
    src/examples/q4_tension_example.cpp
    src/input/abaqus_boundary_parser.cpp
    src/input/abaqus_cload_parser.cpp
    src/input/abaqus_dload_parser.cpp
    src/input/abaqus_element_parser.cpp
    src/input/abaqus_input_file.cpp
    src/input/abaqus_material_parser.cpp
    src/input/abaqus_node_parser.cpp
    src/input/abaqus_node_set_parser.cpp
    src/input/abaqus_q4_model_parser.cpp
    src/input/abaqus_solid_section_parser.cpp
    src/math/dense_matrix.cpp
    src/math/dense_vector.cpp
    src/math/linear_solver.cpp
    src/mechanics/constitutive_matrix.cpp
    src/mechanics/plane_strain_measures.cpp
    src/mechanics/plane_stress_measures.cpp
    src/model/dof_map.cpp
    src/model/isotropic_elastic_material.cpp
    src/model/material_collection.cpp
    src/model/node.cpp
    src/model/node_collection.cpp
    src/model/point_load.cpp
    src/model/q4_edge_pressure_load.cpp
    src/model/q4_element.cpp
    src/model/q4_element_collection.cpp
    src/output/q4_analysis_vtu.cpp
    src/output/q4_vtu_writer.cpp
    src/postprocessing/q4_plane_strain_results.cpp
    src/postprocessing/q4_plane_stress_results.cpp
    src/solver/abaqus_q4_analysis.cpp
    src/solver/abaqus_q4_plane_strain_analysis.cpp
    src/solver/abaqus_q4_plane_stress_analysis.cpp
    src/solver/boundary_conditions.cpp
    src/solver/dense_static_solver.cpp
    src/solver/q4_plane_strain_analysis.cpp
    src/solver/q4_plane_stress_analysis.cpp
    src/solver/reactions.cpp
)

add_library(FinEleMethod::Core ALIAS FinEleMethodCore)

target_compile_features(FinEleMethodCore PUBLIC cxx_std_20)
target_include_directories(FinEleMethodCore
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

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

if(MSVC)
    target_compile_options(FinEleMethodCore PRIVATE /W4 /permissive-)
    target_compile_options(FinEleMethodCli PRIVATE /W4 /permissive-)
    target_compile_options(FinEleMethod PRIVATE /W4 /permissive-)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
