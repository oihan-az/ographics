add_library(orenderer STATIC)

add_library(ographics::orenderer ALIAS orenderer)

target_compile_features(
    orenderer
    PUBLIC
        cxx_std_20
)

target_include_directories(
    orenderer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(
    orenderer
    PRIVATE
        orenderer_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_sources(
    orenderer
    PRIVATE
        src/renderer.cpp
        src/render_pipeline.cpp
        src/render_feature_context.cpp

)

target_link_libraries(
    orenderer
    PUBLIC
        ogfx
        orendergraph
)