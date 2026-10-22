add_executable(quad_compute)

target_sources(
    quad_compute
    PRIVATE
        main.cpp
 )

target_link_libraries(
    quad_compute
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    quad_compute
)