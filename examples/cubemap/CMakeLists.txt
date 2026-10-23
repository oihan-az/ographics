add_executable(cubemap)

target_sources(
    cubemap
    PRIVATE
        main.cpp
 )

target_link_libraries(
    cubemap
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    cubemap
)