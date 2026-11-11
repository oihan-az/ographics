add_executable(omega_egine_demo)

target_sources(
    omega_egine_demo
    PRIVATE
        main.cpp
 )

target_link_libraries(
    omega_egine_demo
    PRIVATE
        ographics::oplatform
        ographics::ogfx
        ographics::orendergraph
        ographics::orenderer
        ographics::omega_engine_core
        omega_engine
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    omega_egine_demo
)