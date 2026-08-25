find_package(GTest CONFIG REQUIRED)

add_executable(FinEleMethodTests
    cli/command_line_test.cpp
    core/application_test.cpp
    core/exit_code_test.cpp
    math/dense_matrix_test.cpp
    math/dense_vector_test.cpp
    math/linear_solver_test.cpp
    model/node_test.cpp
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
