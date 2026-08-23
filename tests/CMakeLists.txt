find_package(GTest CONFIG REQUIRED)

add_executable(FinEleMethodTests
    core/application_test.cpp
)

target_compile_features(FinEleMethodTests PRIVATE cxx_std_20)
target_link_libraries(FinEleMethodTests
    PRIVATE
        FinEleMethod::Core
        GTest::gtest_main
)

if(MSVC)
    target_compile_options(FinEleMethodTests PRIVATE /W4 /permissive-)
endif()

include(GoogleTest)
gtest_discover_tests(FinEleMethodTests)
