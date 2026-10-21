add_executable(triangle_textured)

target_sources(
    triangle_textured
    PRIVATE
        main.cpp
 )

target_link_libraries(
    triangle_textured
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    triangle_textured
)