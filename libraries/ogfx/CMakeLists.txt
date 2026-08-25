add_library(ogfx STATIC)

add_library(ographics::ogfx ALIAS ogfx)

target_compile_features(
    ogfx
    PUBLIC
        cxx_std_20
)

target_include_directories(
    ogfx
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(
    ogfx
    PRIVATE
        OGFX_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_sources(
    ogfx
    PRIVATE
        src/debug.cpp
        
)

set(OGFX_API "VULKAN" CACHE STRING "Graphics API backend")

set_property(
    CACHE OGFX_API
    PROPERTY STRINGS
        VULKAN
        D3D12
        METAL
)

if(OGFX_API STREQUAL "VULKAN")

    add_subdirectory(src/backends/vulkan)

elseif(OGFX_API STREQUAL "D3D12")

    message(FATAL_ERROR "D3D12 backend is not implemented yet.")

elseif(OGFX_API STREQUAL "METAL")

    message(FATAL_ERROR "Metal backend is not implemented yet.")

else()

    message(FATAL_ERROR "Unsupported OGFX_API: ${OGFX_API}")

endif()