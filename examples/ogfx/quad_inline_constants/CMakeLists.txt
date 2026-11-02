add_executable(quad_inline_constants)

target_sources(
    quad_inline_constants
    PRIVATE
        main.cpp
 )

target_link_libraries(
    quad_inline_constants
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    quad_inline_constants
)