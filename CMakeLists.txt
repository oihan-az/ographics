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
add_subdirectory(examples/commands_sync)