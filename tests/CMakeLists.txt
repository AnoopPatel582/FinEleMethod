find_package(GTest CONFIG REQUIRED)

add_executable(FinEleMethodTests
    assembly/load_assembly_test.cpp
    assembly/q4_dof_mapping_test.cpp
    assembly/q4_stiffness_assembly_test.cpp
    cli/command_line_test.cpp
    core/application_test.cpp
    core/exit_code_test.cpp
    elements/q4_gauss_quadrature_test.cpp
    elements/q4_jacobian_test.cpp
    elements/q4_physical_derivatives_test.cpp
    elements/q4_plane_stress_model_stiffness_test.cpp
    elements/q4_plane_stress_stiffness_test.cpp
    elements/q4_shape_functions_test.cpp
    elements/q4_strain_displacement_test.cpp
    math/dense_matrix_test.cpp
    math/dense_vector_test.cpp
    math/linear_solver_test.cpp
    mechanics/constitutive_matrix_test.cpp
    model/dof_map_test.cpp
    model/isotropic_elastic_material_test.cpp
    model/material_collection_test.cpp
    model/node_test.cpp
    model/node_collection_test.cpp
    model/point_load_test.cpp
    model/q4_element_test.cpp
    model/q4_element_collection_test.cpp
    solver/boundary_conditions_test.cpp
    solver/dense_static_solver_test.cpp
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
