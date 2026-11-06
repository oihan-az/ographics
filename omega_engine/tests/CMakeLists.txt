find_package(GTest CONFIG REQUIRED)

add_executable(omega_tests
    scene/scene_tests.cpp

    entity/entity_data_cache_tests.cpp

    core/component_storage_tests.cpp
    core/component_manager_tests.cpp
    core/event_tests.cpp

    transform/transform_tests.cpp
    transform/transform_system_tests.cpp

    camera/camera_tests.cpp
)

target_link_libraries(omega_tests
    PRIVATE 
        omega_engine
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(omega_tests)