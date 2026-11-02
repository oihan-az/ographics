add_executable(orendergraph_quad_compute)

target_sources(
    orendergraph_quad_compute
    PRIVATE
        main.cpp
 )

target_link_libraries(
    orendergraph_quad_compute
    PRIVATE
        ographics::oplatform
        ographics::ogfx
        ographics::orendergraph
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    orendergraph_quad_compute
)