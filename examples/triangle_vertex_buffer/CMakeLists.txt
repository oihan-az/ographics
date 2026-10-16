add_executable(triangle_vertex_buffer)

target_sources(
    triangle_vertex_buffer
    PRIVATE
        main.cpp
 )

target_link_libraries(
    triangle_vertex_buffer
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    triangle_vertex_buffer
)