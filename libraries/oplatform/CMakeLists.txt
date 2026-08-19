add_library(ographics_platform STATIC)

add_library(ographics::platform ALIAS ographics_platform)

target_compile_features(
    ographics_platform
    PUBLIC
        cxx_std_20
)

target_include_directories(
    ographics_platform
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(
    ographics_platform
    PRIVATE
        src/runtime.cpp
        src/window.cpp
)

find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(
    ographics_platform
    PRIVATE
        glfw
)