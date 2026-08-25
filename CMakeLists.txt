cmake_minimum_required(VERSION 3.25)

project(FinEleMethod LANGUAGES CXX)

include(CTest)

add_library(FinEleMethodCore STATIC
    src/core/application.cpp
    src/math/dense_matrix.cpp
    src/math/dense_vector.cpp
    src/math/linear_solver.cpp
    src/model/node.cpp
    src/model/node_collection.cpp
    src/solver/boundary_conditions.cpp
    src/solver/dense_static_solver.cpp
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
