cmake_minimum_required(VERSION 3.25)

project(
    ographics
    VERSION 0.1.0
    DESCRIPTION "A modular, API-agnostic C++ framework for real-time rendering and general-purpose GPU workloads."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Graphics API backend
set(OGRAPHICS_API "VULKAN" CACHE STRING "Graphics API backend")

set_property(
    CACHE OGRAPHICS_API
    PROPERTY STRINGS
        VULKAN
        D3D12
        METAL
)

message(STATUS "Graphics API: ${OGRAPHICS_API}")

add_subdirectory(libraries/oplatform)
add_subdirectory(libraries/ogfx)
add_subdirectory(libraries/orendergraph)
add_subdirectory(libraries/orenderer)

add_subdirectory(examples/ogfx/triangle_shader_data)
add_subdirectory(examples/ogfx/triangle_vertex_buffer)
add_subdirectory(examples/ogfx/triangle_data_buffer)
add_subdirectory(examples/ogfx/quad_textured)
add_subdirectory(examples/ogfx/quad_inline_constants)
add_subdirectory(examples/ogfx/quad_compute)
add_subdirectory(examples/ogfx/cubemap)
add_subdirectory(examples/ogfx/indirect_drawing)
add_subdirectory(examples/ogfx/ray_tracing_triangles)
add_subdirectory(examples/ogfx/ray_tracing_sphere)
add_subdirectory(examples/orendergraph/quad_compute)
add_subdirectory(examples/orendergraph/ray_tracing_triangles)
add_subdirectory(examples/orenderer/demo)

add_subdirectory(omega_engine)

include(CTest)