add_executable(triangle_shader_data)

target_sources(
    triangle_shader_data
    PRIVATE
        main.cpp
 )

target_link_libraries(
    triangle_shader_data
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    triangle_shader_data
)