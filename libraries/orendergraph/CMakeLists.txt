add_library(orendergraph STATIC)

add_library(ographics::orendergraph ALIAS orendergraph)

target_compile_features(
    orendergraph
    PUBLIC
        cxx_std_20
)

target_include_directories(
    orendergraph
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(
    orendergraph
    PRIVATE
        orendergraph_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_sources(
    orendergraph
    PRIVATE
        src/render_graph.cpp
        src/resource_pool_registry.cpp
        src/object_cache.cpp
        src/node_context/pipeline_context.cpp
        src/node_context/raster_context.cpp
        src/node_context/compute_context.cpp
        src/node_context/ray_tracing_context.cpp
        src/node_context/blas_context.cpp
        src/node_context/tlas_context.cpp
        src/node_context/blit_context.cpp

)

target_link_libraries(
    orendergraph
    PUBLIC
        ogfx
)