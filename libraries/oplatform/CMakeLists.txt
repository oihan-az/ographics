add_library(oplatform STATIC)

add_library(ographics::oplatform ALIAS oplatform)

target_compile_features(
    oplatform
    PUBLIC
        cxx_std_20
)

target_include_directories(
    oplatform
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(
    oplatform
    PRIVATE
        OPLATFORM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_sources(
    oplatform
    PRIVATE
        src/debug.cpp
        src/runtime.cpp
        src/glfw_window.cpp
)

find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(
    oplatform
    PRIVATE
        glfw
)

target_link_libraries(
    oplatform
    PUBLIC
        ogfx
)

if(OGRAPHICS_API  STREQUAL "VULKAN")

    add_subdirectory(src/backends/vulkan)

else()

    message(FATAL_ERROR "Unsupported OGRAPHICS_API: ${OGRAPHICS_API}")

endif()