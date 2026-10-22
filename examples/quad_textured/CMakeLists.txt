add_executable(quad_textured)

target_sources(
    quad_textured
    PRIVATE
        main.cpp
 )

target_link_libraries(
    quad_textured
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    quad_textured
)