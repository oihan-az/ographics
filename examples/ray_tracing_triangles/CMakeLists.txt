add_executable(ray_tracing_triangles)

target_sources(
    ray_tracing_triangles
    PRIVATE
        main.cpp
 )

target_link_libraries(
    ray_tracing_triangles
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    ray_tracing_triangles
)