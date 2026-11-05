find_package(GTest CONFIG REQUIRED)

add_executable(omega_tests
    scene/scene_tests.cpp
    core/component_storage_tests.cpp
)

target_link_libraries(omega_tests
    PRIVATE 
        omega_engine
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(omega_tests)