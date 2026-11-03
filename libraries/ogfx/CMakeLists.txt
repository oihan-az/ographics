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

option(OGFX_ENABLE_LOGS "Enable logging for ogfx" ON)

if(OGFX_ENABLE_LOGS)
    message(STATUS "ogfx: Logging ENABLED")

    target_compile_definitions(
        ogfx
        PRIVATE
            OGFX_ENABLE_LOGS
    )
else()
    message(STATUS "ogfx: Logging DISABLED")
endif()

target_sources(
    ogfx
    PRIVATE
        src/debug.cpp
        
)

if(OGRAPHICS_API STREQUAL "VULKAN")

    add_subdirectory(src/backends/vulkan)

else()

    message(FATAL_ERROR "Unsupported OGRAPHICS_API : ${OGRAPHICS_API }")

endif()