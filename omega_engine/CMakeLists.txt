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
        src/platform/platform_system.cpp

        src/transform/matrix_transform.cpp
        src/transform/transform_system.cpp

        src/camera/matrix_camera.cpp
        src/camera/camera_system.cpp

        src/mesh/mesh_asset.cpp
        src/mesh/vertex_layout_registry.cpp
        src/mesh/mesh_page_base.cpp
        src/mesh/mesh_page.cpp  
        src/mesh/mesh_layout_group.cpp
        src/mesh/mesh_system.cpp
        src/mesh/gpu_mesh_page.cpp
        src/mesh/gpu_mesh_layout_group.cpp
        src/mesh/gpu_mesh_system.cpp
        src/mesh/vertex.cpp

        src/texture/texture_asset.cpp
        src/texture/texture_system.cpp
        src/texture/gpu_texture_system.cpp

        src/graphics/graphics_system.cpp

        src/graphics/pipelines/raster_pipeline/raster_pipeline.cpp
        src/graphics/pipelines/raster_pipeline/features/geometry_feature.cpp
        src/graphics/pipelines/raster_pipeline/features/to_swapchain_feature.cpp

        src/graphics/resources/gpu_dynamic_buffer.cpp

        src/loader/loader.cpp
        src/assets/assimp/assimp_model_importer.cpp
        src/assets/texture_loader.cpp

        src/model/model_system.cpp

 )

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)

target_link_libraries(
    omega_engine
    PUBLIC
        ographics::oplatform
        ographics::ogfx
        ographics::orendergraph
        ographics::orenderer
        ographics::omega_engine_core

    PRIVATE
        assimp::assimp
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    omega_engine
)