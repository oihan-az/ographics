add_executable(orendergraph_ray_tracing_triangles)

target_sources(
    orendergraph_ray_tracing_triangles
    PRIVATE
        main.cpp
 )

target_link_libraries(
    orendergraph_ray_tracing_triangles
    PRIVATE
        ographics::oplatform
        ographics::ogfx
        ographics::orendergraph
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    orendergraph_ray_tracing_triangles
)