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

add_subdirectory(libraries/oplatform)
add_subdirectory(libraries/ogfx)
add_subdirectory(examples/instance)