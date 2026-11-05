find_package(GTest CONFIG REQUIRED)

add_executable(omega_tests
    core/scene_tests.cpp
)

target_link_libraries(omega_tests
    PRIVATE 
        omega_engine
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(omega_tests)