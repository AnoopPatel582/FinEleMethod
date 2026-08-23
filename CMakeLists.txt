cmake_minimum_required(VERSION 3.25)

project(FinEleMethod LANGUAGES CXX)

add_executable(FinEleMethod
    src/main.cpp
)

target_compile_features(FinEleMethod PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(FinEleMethod PRIVATE /W4 /permissive-)
endif()

