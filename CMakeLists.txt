cmake_minimum_required(VERSION 3.25)

project(FinEleMethod LANGUAGES CXX)

include(CTest)

add_library(FinEleMethodCore STATIC
    src/core/application.cpp
)

add_library(FinEleMethod::Core ALIAS FinEleMethodCore)

target_compile_features(FinEleMethodCore PUBLIC cxx_std_20)
target_include_directories(FinEleMethodCore
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(FinEleMethod
    apps/cli/main.cpp
)

target_compile_features(FinEleMethod PRIVATE cxx_std_20)
target_link_libraries(FinEleMethod PRIVATE FinEleMethod::Core)

if(MSVC)
    target_compile_options(FinEleMethodCore PRIVATE /W4 /permissive-)
    target_compile_options(FinEleMethod PRIVATE /W4 /permissive-)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
