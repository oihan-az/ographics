find_package(GTest CONFIG REQUIRED)

add_executable(omega_tests
    #"core/entity/entity_system_tests.cpp"
    #"core/hierarchy/hierarchy_system_tests.cpp"
    #"core/component/component_system_tests.cpp"
    #"core/memory/range_allocator_tests.cpp"

    #"transform/transform_system_tests.cpp"
    #"camera/camera_system_tests.cpp"
    #"mesh/mesh_asset_tests.cpp"
    #"mesh/vertex_layout_registry_tests.cpp"
    #"mesh/mesh_page_tests.cpp"
    #"mesh/mesh_layout_group_tests.cpp"
    #"mesh/mesh_system_tests.cpp"
    #"mesh/gpu_mesh_system_tests.cpp"
    #"texture/texture_system_tests.cpp"
    "texture/gpu_texture_system_tests.cpp"
    
)

target_link_libraries(omega_tests
    PRIVATE 
        omega_engine
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(omega_tests)