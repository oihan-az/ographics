add_executable(indirect_drawing)

target_sources(
    indirect_drawing
    PRIVATE
        main.cpp
 )

target_link_libraries(
    indirect_drawing
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    indirect_drawing
)