find_package(GTest CONFIG REQUIRED)

add_executable(omega_tests
    #entity/entity_system_tests.cpp
    #hierarchy/hierarchy_system_tests.cpp
    component/component_system_tests.cpp
)

target_link_libraries(omega_tests
    PRIVATE 
        omega_engine
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(omega_tests)