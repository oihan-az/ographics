add_executable(orenderer_demo)

target_sources(
    orenderer_demo
    PRIVATE
        main.cpp
        scene/geometry_generation.cpp
        loader/loader.cpp
        renderer/pipelines/raster_pipeline.cpp
        renderer/features/deferred_lighting_feature.cpp
        renderer/features/to_swapchain_feature.cpp
 )

target_link_libraries(
    orenderer_demo
    PRIVATE
        ographics::oplatform
        ographics::ogfx
        ographics::orendergraph
        ographics::orenderer
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    orenderer_demo
)