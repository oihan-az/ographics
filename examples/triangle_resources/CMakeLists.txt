add_executable(triangle_resources)

target_sources(
    triangle_resources
    PRIVATE
        main.cpp
 )

target_link_libraries(
    triangle_resources
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    triangle_resources
)