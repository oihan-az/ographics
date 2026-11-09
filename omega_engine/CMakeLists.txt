add_library(omega_engine STATIC)

target_compile_features(
    omega_engine
    PUBLIC
        cxx_std_20
)

target_include_directories(
    omega_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(
    omega_engine
    PRIVATE
        src/transform/matrix_transform.cpp
        src/transform/transform_system.cpp

        src/camera/matrix_camera.cpp
        src/camera/camera_system.cpp

        src/mesh/mesh_asset.cpp
        src/mesh/vertex_layout_registry.cpp
        src/mesh/mesh_page.cpp
        src/mesh/mesh_layout_group.cpp
        src/mesh/mesh_system.cpp

 )

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

target_link_libraries(
    omega_engine
    PUBLIC
        ographics::oplatform
        ographics::ogfx
        ographics::orendergraph
        ographics::orenderer
        ographics::omega_engine_core
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    omega_engine
)