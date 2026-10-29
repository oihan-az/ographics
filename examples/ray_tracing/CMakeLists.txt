add_executable(ray_tracing)

target_sources(
    ray_tracing
    PRIVATE
        main.cpp
 )

target_link_libraries(
    ray_tracing
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    ray_tracing
)